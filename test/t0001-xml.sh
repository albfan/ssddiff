#!/bin/sh

test_description="Diffting xml files"

. ./setup.sh

test_expect_success "diff two xml files" '
echo $PWD > eco
echo $DIR_TEST > brio

   $SHARNESS_BUILD_DIRECTORY/src/xmldiff $DIR_TEST/operations1.xml $DIR_TEST/operations2.xml > output.xml
   diff output.xml $DIR_TEST/result.xml
'

test_done
