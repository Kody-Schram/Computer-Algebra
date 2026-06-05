#!/bin/bash

for config in *.yaml; do
    valgrind --leak-check=full -s ../build/algebra "$config"

    CODE=$?

    if [ $CODE -ne 0 ]; then
        break
    fi
done
