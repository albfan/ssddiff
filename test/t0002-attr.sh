#!/bin/sh

test_description="Diffting xml attributes"

. ./setup.sh

test_expect_success "diff xml attributes" '
   $SHARNESS_BUILD_DIRECTORY/src/xmldiff $DIR_TEST/ops-attr1.xml $DIR_TEST/ops-attr2.xml > output.xml
   diff output.xml $DIR_TEST/result.xml
'

test_done
