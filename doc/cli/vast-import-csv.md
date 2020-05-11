The `vast import csv` command imports [comma-separated
values](https://en.wikipedia.org/wiki/Comma-separated_values) in tabular form.
The first line in a CSV file must contain a header that describes the field
names. The remaining lines contain concrete values. Except for the header, one
line corresponds to one event.

Because CSV has no notion of typing, it is necessary to select a layout via
`--type`/`-t` whose field names correspond to the CSV header field names. Such a
layout must either be defined in a schema file known to VAST, or be defined in a
schema passed using `--schema` / `-S` or `--schema-file` / `-s`.

E.g., to import Threat Intelligence data into VAST, the known type
`intel.indicator` can be used:

```sh
vast import csv --type=intel.indicator -r path/to/indicators.csv
```
