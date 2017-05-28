#!/usr/bin/env bash

documentation build include/shelf-pack.hpp \
    --name shelf-pack-cpp \
    --project-version 2.0.1 \
    --polyglot --parseExtension=hpp \
    --lint \
    --github \
    --format html \
    --output docs/
