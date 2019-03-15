#!/bin/bash
#
# ~~~
# This file is part of the dune-gdt project:
#   https://github.com/dune-community/dune-gdt
# Copyright 2010-2018 dune-gdt developers and contributors. All rights reserved.
# License: Dual licensed as BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)
#      or  GPL-2.0+ (http://opensource.org/licenses/gpl-license)
#          with "runtime exception" (http://www.dune-project.org/license.html)
# Authors:
#   Felix Schindler (2017)
#   René Fritze     (2016 - 2018)
#   Tobias Leibner  (2018)
# ~~~

set -ex


MY_BUILD_DIR=${DUNE_BUILD_DIR}/${MY_MODULE}

cd ${SUPERDIR}
${SRC_DCTRL} ${BLD} --only=${MY_MODULE} configure
${SRC_DCTRL} ${BLD} --only=${MY_MODULE} make
${SRC_DCTRL} ${BLD} --only=${MY_MODULE} bexec ninja -v -j 1 check

# if [ "X${TRAVIS_PULL_REQUEST}" != "Xfalse" ] ; then
#         ${SUPERDIR}/.travis/init_sshkey.sh ${encrypted_95fb78800815_key} ${encrypted_95fb78800815_iv} keys/dune-community/dune-gdt-testlogs
# source ${SUPERDIR}/scripts/bash/retry_command.bash
#         retry_command ${SUPERDIR}/scripts/bash/travis_upload_test_logs.bash ${MY_BUILD_DIR}/dune/gdt/test/
# fi

# clang coverage currently disabled for being too mem hungry
if [[ ${CC} == *"clang"* ]] ; then
    exit 0
fi

if [[ ${DRONE_BUILD_EVENT} != "push" ]] ; then
    exit 0
fi

pushd ${MY_BUILD_DIR}
COVERAGE_INFO=${PWD}/coverage.info
lcov --directory . --output-file ${COVERAGE_INFO} -c
for d in "dune-common" "dune-pybindxi" "dune-geometry"  "dune-istl"  "dune-grid" "dune-alugrid"  "dune-uggrid"  "dune-localfunctions" \
         "dune-xt-common" "dune-xt-functions" "dune-xt-la" "dune-xt-grid" ; do
    lcov --directory . --output-file ${COVERAGE_INFO} -r ${COVERAGE_INFO} "${SUPERDIR}/${d}/*"
done
lcov --directory . --output-file ${COVERAGE_INFO} -r ${COVERAGE_INFO} "${SUPERDIR}/${MY_MODULE}/dune/xt/*/test/*"
cd ${SUPERDIR}/${MY_MODULE}
${MY_BUILD_DIR}/run-in-dune-env pip install --no-cache codecov
${MY_BUILD_DIR}/run-in-dune-env codecov -v -X gcov -X coveragepy -F ctest -f ${COVERAGE_INFO} -t ${CODECOV_TOKEN}
popd
