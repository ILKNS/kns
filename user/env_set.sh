#!/bin/bash

sudo sh -c 'for i in /sys/devices/system/node/node*/hugepages/hugepages-2048kB/nr_hugepages; do echo 4096 > $i; done'
mkdir /mnt/huge
mount -t hugetlbfs -o size=2m,nr_inodes=4096 none /mnt/huge
sudo ./hugepage_user.o