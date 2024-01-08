#include <idx.h>

module_index_t toIdxType(subEventIdxType st, module_index_t idx)
{
  auto from=idx.type;
  auto pc_id=idx.pc_id;
  auto sfp=idx.sfp;
  auto mod=idx.mod;
  assert(st>from);
  switch(st)
    {
    case fbxChannelIdx:
      assert(0);
      break;
    case fbxModuleIdx:
      return module_index_t{st, pc_id, sfp, mod, IDX_WILDCARD};
      break;
    default:
      assert(0);
      return IDX_INVALID;
    }
  return IDX_INVALID;
}

