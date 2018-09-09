# swebs
Simple web server -- Simple HTTP server on your local machine.

## Usage

**./swebs [--port=PORT] [--dir=DIR] [--log=DIR] [--help]**

| Option    | Description                                                             | Type                 |
|:---------:|:-----------------------------------------------------------------------:|:--------------------:|
| --port    | Port in which swebs will be listening for incomming connections         | Optional(Default:80) |
| --dir     | Directory that will be used as root directory to swebs to serve files.  | Optional(Default: .) |
| --log     | Directory that will be used to save swebs log file.                     | Optional(Default: .) |
| --help    | Prints help and exit                                                    | Optional             |

_Please note that for certain ports(e.g, 80) you will need root permissions_

## Features

* **GET method**: swebs just supports HTTP GET method

* **Multithreading**: swebs will launch a new thread to process each request separately, using _pthreads_

* **Logging**: swebs will write to a log file(located in the directory specified with _--log_ or at '.' if not directory was specified)
