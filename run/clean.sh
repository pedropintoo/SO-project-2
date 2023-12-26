#!/bin/bash

rm -f error*
rm -f core

# change 0x61066137 to your semaphore and shared memory key
# Jorge: 0x610683c9
# Pedro: 0x6106435e

ipcrm -S 0x6106435e
ipcrm -M 0x6106435e
