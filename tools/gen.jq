# Common functions for generating code with JQ.

def _lines_to_string(indent):
  map(
    if (. | type) == "string" then
      indent + . + "\n"
    elif (. | type) == "array" then
       . | _lines_to_string(indent + "  ")
    else
      error("Line is neither a string nor an array.")
    end
  ) | add;

def lines_to_string: _lines_to_string("");

def remove_prefix(prefix):
  if . | startswith(prefix) then
    .[(prefix|length):]
  else
    error("String does not start with the prefix.")
  end;

def hex_dword:
  . as $num | "0x" + (
    [range(0; 32; 4)
      | (["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"][($num / ((28 - .)|exp2))|floor % 16])]
    | join("")
  ) + "U";

def chunk(num; sep): [.[]] as $d | range(0; $d|length; num) | (($d[.:.+num] | join(sep)) + sep);

def snake_case: gsub("(?<=[a-z])(?=[A-Z])"; "_") | ascii_downcase;
