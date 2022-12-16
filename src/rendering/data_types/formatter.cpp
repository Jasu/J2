#include "rendering/data_types/data_type.hpp"
#include "strings/formatters/enum_formatter.hpp"

namespace j::rendering::data_types {
  namespace {
    namespace s = strings;
    J_A(ND, NODESTROY)const s::formatters::enum_formatter<data_type> data_type_formatter{{
      { data_type::none,       "void"       },

      { data_type::u8,         "u8"         },
      { data_type::u16,        "u16"        },
      { data_type::u32,        "u32"        },
      { data_type::u64,        "u64"        },

      { data_type::s8,         "s8"         },
      { data_type::s16,        "s16"        },
      { data_type::s32,        "s32"        },
      { data_type::s64,        "s64"        },

      { data_type::fp16,       "fp16"       },
      { data_type::fp32,       "fp32"       },
      { data_type::fp64,       "fp64"       },

      { data_type::boolean,    "bool"       },



      { data_type::vec2u8,     "vec2u8"     },
      { data_type::vec2u16,    "vec2u16"    },
      { data_type::vec2u32,    "vec2u32"    },
      { data_type::vec2u64,    "vec2u64"    },

      { data_type::vec2s8,     "vec2s8"     },
      { data_type::vec2s16,    "vec2s16"    },
      { data_type::vec2s32,    "vec2s32"    },
      { data_type::vec2s64,    "vec2s64"    },

      { data_type::vec2fp16,   "vec2fp16"   },
      { data_type::vec2fp32,   "vec2fp32"   },
      { data_type::vec2fp64,   "vec2fp64"   },

      { data_type::vec2b,      "vec2b"      },



      { data_type::vec3u8,     "vec3u8"     },
      { data_type::vec3u16,    "vec3u16"    },
      { data_type::vec3u32,    "vec3u32"    },
      { data_type::vec3u64,    "vec3u64"    },

      { data_type::vec3s8,     "vec3s8"     },
      { data_type::vec3s16,    "vec3s16"    },
      { data_type::vec3s32,    "vec3s32"    },
      { data_type::vec3s64,    "vec3s64"    },

      { data_type::vec3fp16,   "vec3fp16"   },
      { data_type::vec3fp32,   "vec3fp32"   },
      { data_type::vec3fp64,   "vec3fp64"   },

      { data_type::vec3b,      "vec3b"      },


      { data_type::vec4u8,     "vec4u8"     },
      { data_type::vec4u16,    "vec4u16"    },
      { data_type::vec4u32,    "vec4u32"    },
      { data_type::vec4u64,    "vec4u64"    },

      { data_type::vec4s8,     "vec4s8"     },
      { data_type::vec4s16,    "vec4s16"    },
      { data_type::vec4s32,    "vec4s32"    },
      { data_type::vec4s64,    "vec4s64"    },

      { data_type::vec4fp16,   "vec4fp16"   },
      { data_type::vec4fp32,   "vec4fp32"   },
      { data_type::vec4fp64,   "vec4fp64"   },

      { data_type::vec4b,      "vec4b"      },


      { data_type::mat2x2u8,   "mat2x2u8"   },
      { data_type::mat2x2u16,  "mat2x2u16"  },
      { data_type::mat2x2u32,  "mat2x2u32"  },
      { data_type::mat2x2u64,  "mat2x2u64"  },

      { data_type::mat2x2s8,   "mat2x2s8"   },
      { data_type::mat2x2s16,  "mat2x2s16"  },
      { data_type::mat2x2s32,  "mat2x2s32"  },
      { data_type::mat2x2s64,  "mat2x2s64"  },

      { data_type::mat2x2fp16, "mat2x2fp16" },
      { data_type::mat2x2fp32, "mat2x2fp32" },
      { data_type::mat2x2fp64, "mat2x2fp64" },

      { data_type::mat2x2b,    "mat2x2b"      },



      { data_type::mat3x2u8,   "mat3x2u8"   },
      { data_type::mat3x2u16,  "mat3x2u16"  },
      { data_type::mat3x2u32,  "mat3x2u32"  },
      { data_type::mat3x2u64,  "mat3x2u64"  },

      { data_type::mat3x2s8,   "mat3x2s8"   },
      { data_type::mat3x2s16,  "mat3x2s16"  },
      { data_type::mat3x2s32,  "mat3x2s32"  },
      { data_type::mat3x2s64,  "mat3x2s64"  },

      { data_type::mat3x2fp16, "mat3x2fp16" },
      { data_type::mat3x2fp32, "mat3x2fp32" },
      { data_type::mat3x2fp64, "mat3x2fp64" },

      { data_type::mat3x2b,    "mat3x2b"    },



      { data_type::mat4x2u8,   "mat4x2u8"   },
      { data_type::mat4x2u16,  "mat4x2u16"  },
      { data_type::mat4x2u32,  "mat4x2u32"  },
      { data_type::mat4x2u64,  "mat4x2u64"  },

      { data_type::mat4x2s8,   "mat4x2s8"   },
      { data_type::mat4x2s16,  "mat4x2s16"  },
      { data_type::mat4x2s32,  "mat4x2s32"  },
      { data_type::mat4x2s64,  "mat4x2s64"  },

      { data_type::mat4x2fp16, "mat4x2fp16" },
      { data_type::mat4x2fp32, "mat4x2fp32" },
      { data_type::mat4x2fp64, "mat4x2fp64" },

      { data_type::mat4x2b,    "mat4x2b"    },



      { data_type::mat2x3u8,   "mat2x3u8"   },
      { data_type::mat2x3u16,  "mat2x3u16"  },
      { data_type::mat2x3u32,  "mat2x3u32"  },
      { data_type::mat2x3u64,  "mat2x3u64"  },

      { data_type::mat2x3s8,   "mat2x3s8"   },
      { data_type::mat2x3s16,  "mat2x3s16"  },
      { data_type::mat2x3s32,  "mat2x3s32"  },
      { data_type::mat2x3s64,  "mat2x3s64"  },

      { data_type::mat2x3fp16, "mat2x3fp16" },
      { data_type::mat2x3fp32, "mat2x3fp32" },
      { data_type::mat2x3fp64, "mat2x3fp64" },

      { data_type::mat2x3b,    "mat2x3b"    },



      { data_type::mat3x3u8,   "mat3x3u8"   },
      { data_type::mat3x3u16,  "mat3x3u16"  },
      { data_type::mat3x3u32,  "mat3x3u32"  },
      { data_type::mat3x3u64,  "mat3x3u64"  },

      { data_type::mat3x3s8,   "mat3x3s8"   },
      { data_type::mat3x3s16,  "mat3x3s16"  },
      { data_type::mat3x3s32,  "mat3x3s32"  },
      { data_type::mat3x3s64,  "mat3x3s64"  },

      { data_type::mat3x3fp16, "mat3x3fp16" },
      { data_type::mat3x3fp32, "mat3x3fp32" },
      { data_type::mat3x3fp64, "mat3x3fp64" },

      { data_type::mat3x3b,    "mat3x3b"    },


      { data_type::mat4x3u8,   "mat4x3u8"   },
      { data_type::mat4x3u16,  "mat4x3u16"  },
      { data_type::mat4x3u32,  "mat4x3u32"  },
      { data_type::mat4x3u64,  "mat4x3u64"  },

      { data_type::mat4x3s8,   "mat4x3s8"   },
      { data_type::mat4x3s16,  "mat4x3s16"  },
      { data_type::mat4x3s32,  "mat4x3s32"  },
      { data_type::mat4x3s64,  "mat4x3s64"  },

      { data_type::mat4x3fp16, "mat4x3fp16" },
      { data_type::mat4x3fp32, "mat4x3fp32" },
      { data_type::mat4x3fp64, "mat4x3fp64" },

      { data_type::mat4x3b,    "mat4x3b"    },


      { data_type::mat2x4u8,   "mat2x4u8"   },
      { data_type::mat2x4u16,  "mat2x4u16"  },
      { data_type::mat2x4u32,  "mat2x4u32"  },
      { data_type::mat2x4u64,  "mat2x4u64"  },

      { data_type::mat2x4s8,   "mat2x4s8"   },
      { data_type::mat2x4s16,  "mat2x4s16"  },
      { data_type::mat2x4s32,  "mat2x4s32"  },
      { data_type::mat2x4s64,  "mat2x4s64"  },

      { data_type::mat2x4fp16, "mat2x4fp16" },
      { data_type::mat2x4fp32, "mat2x4fp32" },
      { data_type::mat2x4fp64, "mat2x4fp64" },

      { data_type::mat2x4b,    "mat2x4b"    },


      { data_type::mat3x4u8,   "mat3x4u8"   },
      { data_type::mat3x4u16,  "mat3x4u16"  },
      { data_type::mat3x4u32,  "mat3x4u32"  },
      { data_type::mat3x4u64,  "mat3x4u64"  },

      { data_type::mat3x4s8,   "mat3x4s8"   },
      { data_type::mat3x4s16,  "mat3x4s16"  },
      { data_type::mat3x4s32,  "mat3x4s32"  },
      { data_type::mat3x4s64,  "mat3x4s64"  },

      { data_type::mat3x4fp16, "mat3x4fp16" },
      { data_type::mat3x4fp32, "mat3x4fp32" },
      { data_type::mat3x4fp64, "mat3x4fp64" },

      { data_type::mat3x4b,    "mat3x4b"    },


      { data_type::mat4x4u8,   "mat4x4u8"   },
      { data_type::mat4x4u16,  "mat4x4u16"  },
      { data_type::mat4x4u32,  "mat4x4u32"  },
      { data_type::mat4x4u64,  "mat4x4u64"  },

      { data_type::mat4x4s8,   "mat4x4s8"   },
      { data_type::mat4x4s16,  "mat4x4s16"  },
      { data_type::mat4x4s32,  "mat4x4s32"  },
      { data_type::mat4x4s64,  "mat4x4s64"  },

      { data_type::mat4x4fp16, "mat4x4fp16" },
      { data_type::mat4x4fp32, "mat4x4fp32" },
      { data_type::mat4x4fp64, "mat4x4fp64" },

      { data_type::mat4x4b,    "mat4x4b"    },
    }};
  }
}
