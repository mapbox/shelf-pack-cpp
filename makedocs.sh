#!/usr/bin/env bash

documentation build include/mapbox/shelf-pack.hpp \
    --name shelf-pack-cpp \
    --project-version 2.1.1 \
    --polyglot --parseExtension=hpp \
    --lint \
    --github \
    --format html \
    --output docs/
