#include <cstddef>
#include <cstdint>

namespace {

struct Header {
  Header *next;
  size_t size;
};

constexpr size_t POOL_SIZE = 16384;
alignas(alignof(max_align_t)) static char pool[POOL_SIZE];
static Header *freeList = nullptr;

static_assert(sizeof(Header) <= alignof(max_align_t));

void init()
{
  if (freeList != nullptr) {
    return;
  }

  freeList = reinterpret_cast<Header *>(pool);
  freeList->next = nullptr;
  freeList->size = POOL_SIZE - sizeof(Header);
}

} // anonymous namespace

extern "C" void *malloc(size_t size)
{
  if (size == 0) {
    return nullptr;
  }

  init();

  const size_t aligned = (size + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1);
  const size_t needed = sizeof(Header) + aligned;

  Header *prev = nullptr;
  Header *cur = freeList;
  while (cur != nullptr) {
    if (cur->size >= needed) {
      if (prev != nullptr) {
        prev->next = cur->next;
      }
      else {
        freeList = cur->next;
      }
      return reinterpret_cast<char *>(cur) + sizeof(Header);
    }

    prev = cur;
    cur = cur->next;
  }

  return nullptr;
}

extern "C" void free(void *ptr)
{
  if (ptr == nullptr) {
    return;
  }

  Header *hdr = reinterpret_cast<Header *>(static_cast<char *>(ptr) - sizeof(Header));
  hdr->next = freeList;
  freeList = hdr;
}
