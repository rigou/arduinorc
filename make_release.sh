#!/bin/bash

cd "$(dirname $0)"
tar czf ../arduinorc_$(date '+%Y-%m-%dT%H%M%S').tgz arduinotx linux
