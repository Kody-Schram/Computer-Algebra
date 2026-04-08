#!/bin/bash

for config in *.yaml; do
    time ../build/algebra "$config"

    CODE=$?

    if [ $CODE -ne 0 ]; then
        break
    fi
done