#include "vast/partition.h"

#include "vast/event.h"
#include "vast/event_index.h"
#include "vast/logger.h"
#include "vast/io/serialization.h"

using namespace cppa;

namespace vast {

namespace {

} // namespace <anonymous>

partition::partition(path dir)
  : dir_{std::move(dir)},
    last_modified_{now()}
{
}

void partition::init()
{
  VAST_LOG_ACT_VERBOSE("partition", "spawned");

  if (! exists(dir_))
  {
    VAST_LOG_ACT_DEBUG("partition", "creates new directory " << dir_);
    mkdir(dir_);
  }

  if (exists(dir_ / "last_modified"))
    io::unarchive(dir_ / "last_modified", last_modified_);

  event_meta_index_ = spawn<event_meta_index>(dir_ / "meta");

  become(
      on(atom("kill")) >> [=]
      {
        event_meta_index_ << last_dequeued();
        for (auto& p : event_arg_indexes_)
          p.second << last_dequeued();
        quit();
      },
      on(atom("meta"), atom("timestamp")) >> [=]
      {
        reply(last_modified_);
      },
      on(atom("lookup"), atom("type"), arg_match) >>
        [=](regex const& events, relational_operator op,
            value const& v, actor_ptr sink)
      {
        auto t = make_any_tuple(atom("lookup"), atom("type"), op, v, sink);
        for (auto& p : event_arg_indexes_)
          if (events.match(p.first))
            p.second << t;
      },
      on(atom("lookup"), atom("offset"), arg_match) >>
        [=](regex const& events, relational_operator op,
            value const& v, offset const& off, actor_ptr sink)
      {
        auto t = make_any_tuple(
            atom("lookup"), atom("offset"), op, off, v, sink);

        for (auto& p : event_arg_indexes_)
          if (events.match(p.first))
            p.second << t;
      },
//      on(atom("lookup"), atom("tag"), atom("name"), arg_match) >>
//        [=](relational_operator op, value const& v)
//      {
//        // TODO.
//      },
//      on(atom("lookup"), atom("tag"), atom("timestamp"), arg_match) >>
//        [=](relational_operator op, value const& v)
//      {
//        // TODO.
//      },
      on_arg_match >> [=](event const& e)
      {
        last_modified_ = now();
        event_meta_index_ << last_dequeued();

        auto& a = event_arg_indexes_[e.name()];
        if (! a)
          a = spawn<event_arg_index>(dir_ / "event"/ e.name());
        a << last_dequeued();
      });
}

void partition::on_exit()
{
  VAST_LOG_ACT_DEBUG(
      "partition", "saves last modification time " << last_modified_);
  io::archive(dir_ / "last_modified", last_modified_);
  VAST_LOG_ACT_VERBOSE("partition", "terminated");
}

} // namespace vast
