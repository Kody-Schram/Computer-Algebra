#!/bin/bash

for config in *.yaml; do
    valgrind --leak-check=full ../build/algebra "$config"

    CODE=$?

    if [ $CODE -ne 0 ]; then
        break
    else
        echo "Passed test"
    fi
done