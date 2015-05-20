#!/bin/bash

######################################################################
#
#  Licensed Materials - Property of IBM
#  
#  Restricted Materials of IBM 
#  
#  IBM Informix Dynamic Server
#  (c) Copyright IBM Corporation 2013 All rights reserverd.
# 
# 
#   Title            : TSL_FileLoad
#   Description      : Shell script to load a file
# 
######################################################################

# Simple example of using the 'FILE:' interface to TSL_Put
# Be careful of large files using up memory and log space

db=$1
table=$2
column=$3
file=$4
commits=${5:-3600}

[ $# -lt 5 ] && {
    echo "Usage:"
    echo "      $0 database table column input_file [ commits ]"
}


echo "ARGS: " $# 
