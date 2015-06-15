#include <iostream>
#include <chrono>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "vast/logger.h"
#include "vast/actor/http_broker.h"

#include "vast/actor/actor.h"
#include "vast/event.h"
#include "vast/time.h"
#include "vast/uuid.h"
#include "vast/util/json.h"


// FIXME: remove after debugging
using std::cout;
using std::cerr;
using std::endl;

using namespace caf;
using namespace caf::io;
using namespace std::string_literals;

namespace vast {

const char HEX2DEC[256] =
{
    /*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
    /* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,

    /* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

    /* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,

    /* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};

std::string UriDecode(const std::string & sSrc)
{
    // Note from RFC1630:  "Sequences which start with a percent sign
    // but are not followed by two hexadecimal characters (0-9, A-F) are reserved
    // for future extension"

    const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
	const int SRC_LEN = sSrc.length();
    const unsigned char * const SRC_END = pSrc + SRC_LEN;
    const unsigned char * const SRC_LAST_DEC = SRC_END - 2;   // last decodable '%'

    char * const pStart = new char[SRC_LEN];
    char * pEnd = pStart;

    while (pSrc < SRC_LAST_DEC)
	{
		if (*pSrc == '%')
        {
            char dec1, dec2;
            if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
                && -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
            {
                *pEnd++ = (dec1 << 4) + dec2;
                pSrc += 3;
                continue;
            }
        }

        *pEnd++ = *pSrc++;
	}

    // the last 2- chars
    while (pSrc < SRC_END)
        *pEnd++ = *pSrc++;

    std::string sResult(pStart, pEnd);
    delete [] pStart;
	return sResult;
}


bool first_event_ = true;

template <size_t Size>
constexpr size_t cstr_size(const char (&)[Size])
{
  return Size;
}


std::string parse_url(new_data_msg msg)
{
  std::string bufstr(msg.buf.begin(), msg.buf.end());
  //aout(self) << bufstr << endl;
  auto pos = bufstr.find(' ');
  auto url = bufstr.substr(pos + 1, bufstr.find(' ', pos+1)-4);
  //aout(self) << "url:'" << url << "'" << endl;
  return url;
}

std::string create_response(std::string const& content)
{
  auto response = ""s;
  response += "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: application/json\r\n";
  response += "\r\n";
  response += content;
  response += "\r\n";
  return response;
}

void setup_exporter(broker* self, std::string query, actor const& node, std::string exporter_label){
  //TODO: Use label instead of just "exporter"
  caf::message_builder mb;
  mb.append("spawn");

  mb.append("exporter");
  //TODO: un-hardcode the query opts
  mb.append("-h");
  mb.append("-l 10");
  mb.append(query);
  self->send(node, mb.to_message());

  mb.clear();
  mb.append("connect");
  mb.append("exporter");
  //TODO: Actually get the archive(s) actor and use proper labels
  mb.append("archive");
  self->send(node, mb.to_message());

  mb.clear();
  mb.append("connect");
  mb.append("exporter");
  //TODO: Actually get the index(es) actor and use proper labels
  mb.append("index");
  self->send(node, mb.to_message());

  mb.clear();
  mb.append(get_atom::value);
  mb.append("exporter");
  self->send(node, mb.to_message());
}

bool handle(event const& e, broker* self, connection_handle hdl)
{
  auto j = to<util::json>(e);
  if (! j)
    return false;
  auto content = to_string(*j, true);
  content += "\r\n";

  if (first_event_){
    first_event_ = false;
    auto ans = create_response(content);
    VAST_DEBUG("Sending first event", ans);
    ans = "[" + ans;
    self->write(hdl, ans.size(), ans.c_str());
    self->flush(hdl);
  } 
  else
  {
    content = "," + content;
    self->write(hdl, content.size(), content.c_str());
    self->flush(hdl);
  }
  return true;
}

behavior connection_worker(broker* self, connection_handle hdl, actor const& node)
{
  self->configure_read(hdl, receive_policy::at_most(1024));

  //TODO: make this unique for each connection worker.. maybe use self->id()
  std::string exporter_label = "exporter";

  return
  {
    [=](new_data_msg const& msg)
    {
      auto url = parse_url(msg);
      auto query = url.substr(url.find("query=") + 6, url.size());
      query = UriDecode(query);
      VAST_DEBUG(self, "got", query, "as query");
      setup_exporter(self, query, node, exporter_label);
    },
    [=](connection_closed_msg const&)
    {
      self->quit();
    },
    [=](actor const& act, std::string const& fqn, std::string const& type)
    {

      VAST_DEBUG("got actor", act, "with fqn", fqn, " and type", type);
      if(type == exporter_label)
      {
        //Register at exporter
        caf::message_builder mb;
        mb.append(put_atom::value);
        mb.append(sink_atom::value);
        mb.append(self);
        self->send(act, mb.to_message());
        
        //Run exporter, maybe delay here... if exporter gets run signal before we register we might
        //not get some events
        mb.clear();
        mb.append("send");
        mb.append(exporter_label);
        mb.append("run");
        self->send(node, mb.to_message());
      }
    },
    // handle sink messages
    [=](exit_msg const& msg)
    {
      self->quit(msg.reason);
    },
    [=](uuid const&, event const& e)
    {
      handle(e, self, hdl);
    },
    [=](uuid const&, std::vector<event> const& v)
    {
      assert(! v.empty());
      for (auto& e : v)
        if (! handle(e, self, hdl))
          return;
    },
    [=](uuid const& id, done_atom, time::extent runtime)
    {
      VAST_VERBOSE(self, "got DONE from query", id << ", took", runtime);
      self->write(hdl, 1, "]");
      self->flush(hdl);
      self->quit(exit::done);
    }
  };
}

behavior http_broker_function(broker* self, actor const& node)
{
  VAST_VERBOSE("http_broker_function called");
  return
  {
    [=](new_connection_msg const& ncm)
    {
      VAST_DEBUG(self, "got new connection");
      auto worker = self->fork(connection_worker, ncm.handle, node);
      self->monitor(worker);
    },
    others >> [=]
    {
      auto msg = to_string(self->current_message());
      VAST_WARN(self, "got unexpected msg:", msg);
    }
  };
}

optional<uint16_t> as_u16(std::string const& str)
{
  return static_cast<uint16_t>(stoul(str));
}

} // namespace vast
