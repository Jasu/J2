#pragma once

namespace j::properties {
  enum class PROPERTY_TYPE {
    EMPTY = 0,

    NIL,
    STRING,
    INT,
    BOOL,
    FLOAT,

    LIST,
    MAP,
    SET,
    OBJECT,
    ENUM,
  };
}
