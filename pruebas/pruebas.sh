#!/usr/bin/env bash

set -eu

PROGRAMA="$1"
RET=0

# Correr con diff y sin Valgrind.
for x in *_in; do
  b=${x%_in}
  $PROGRAMA <${b}_in ${b}_doctores ${b}_pacientes | diff -u ${b}_out - || RET=$?
done

if [[ $RET -eq 0 ]]; then
  # Si no hubo errores de diff, correr con Valgrind.
  for x in *_in; do
      b=${x%_in}
      valgrind --leak-check=full --track-origins=yes --error-exitcode=2 \
          $PROGRAMA <${b}_in ${b}_doctores ${b}_pacientes >/dev/null || RET=$?
  done
fi

if [[ $RET -eq 0 ]]; then
  echo "OK"
else
  echo "ERROR"
  exit $RET
fi
