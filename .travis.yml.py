#!/usr/bin/env python3

tpl = '''# This file is part of the dune-gdt project:
#   https://github.com/dune-community/dune-gdt
# Copyright 2010-2016 dune-gdt developers and contributors. All rights reserved.
# License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)
# Authors:
#   Felix Schindler (2016)

# THIS FILE IS AUTOGENERATED -- DO NOT EDIT #


sudo: required
dist: trusty
language: generic
services: docker

before_script:
    - export IMAGE="dunecommunity/${MY_MODULE}-testing:${DOCKER_TAG}_${TRAVIS_BRANCH}"
    # get image with fallback to master branch of the super repo
    - docker pull ${IMAGE} || export IMAGE="dunecommunity/${MY_MODULE}-testing:${DOCKER_TAG}_master" ; docker pull ${IMAGE}
    - export ENV_FILE=${HOME}/env
    - printenv | \grep TRAVIS > ${ENV_FILE}
    - printenv | \grep encrypt >> ${ENV_FILE}
    - printenv | \grep TEST >> ${ENV_FILE}
    - printenv | \grep TOKEN >> ${ENV_FILE}
    - export DOCKER_RUN="docker run --env-file ${ENV_FILE} -v ${TRAVIS_BUILD_DIR}:/root/src/${MY_MODULE} ${IMAGE}"

script:
    - ${DOCKER_RUN} /root/src/${MY_MODULE}/.travis.script.bash

# runs independent of 'script' failure/success
after_script:
    - ${DOCKER_RUN} /root/src/${MY_MODULE}/.travis.after_script.bash

notifications:
  email:
    on_success: change
    on_failure: change
    on_start: never
  webhooks:
    urls:
      - https://buildtimetrend.herokuapp.com/travis
      - https://webhooks.gitter.im/e/2a38e80d2722df87f945

branches:
  except:
    - gh-pages

env:
  global:
    - MY_MODULE=dune-gdt

matrix:
  include:
#   gcc 5
{%- for c in builders %}
    - env: DOCKER_TAG=gcc-5 TESTS={{c}}
{%- endfor %}
#   clang 3.9
{%- for c in builders %}
    - env: DOCKER_TAG=clang-3.9 TESTS={{c}}
{%- endfor %}

# THIS FILE IS AUTOGENERATED -- DO NOT EDIT #
'''

import os
import jinja2
import sys
tpl = jinja2.Template(tpl)
builder_count = int(sys.argv[1])
with open(os.path.join(os.path.dirname(__file__), '.travis.yml'), 'wt') as yml:
    yml.write(tpl.render(builders=range(0, builder_count)))
