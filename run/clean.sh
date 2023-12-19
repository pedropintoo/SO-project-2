#!/bin/bash

rm -f error*
rm -f core

# change 0x61066137 to your semaphore and shared memory key
ipcrm -S 0x610683c9
ipcrm -M 0x610683c9

