bits 64

global ret_fixture:data

%include "macros.mac"

%macro do 0-1+.nolist
  retq   %1
  db 1
  align 16, db 0
%endmacro

ret_fixture:
  do
  do 0
  do 1
  do 2
  do 3
  do 1024
