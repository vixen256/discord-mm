#include "diva.h"

namespace diva {
FUNCTION_PTR (void *, operatorNew, 0x1409777D0, u64);
FUNCTION_PTR (void *, operatorDelete, 0x1409B1E90, void *);
FUNCTION_PTR (void, FreeString, 0x14014BCD0, string *);
FUNCTION_PTR (void, DefaultSprArgs, 0x1405B78D0, SprArgs *args);
FUNCTION_PTR (SprArgs *, DrawSpr, 0x1405B49C0, SprArgs *args);

vector<PvDbEntry *> *pvs        = (vector<PvDbEntry *> *)0x141753818;
map<i32, PvSpriteId> *pvSprites = (map<i32, PvSpriteId> *)0x14CBBACC0;

std::optional<PvDbEntry **>
getPvDbEntry (i32 id) {
	return pvs->find ([=] (auto pv) { return (*pv)->id == id; });
}

std::optional<PvDbDifficulty *>
getPvDbDifficulty (i32 id, i32 difficulty, bool extra) {
	if (id < 1 || difficulty < 0 || difficulty > 4) return nullptr;
	if (auto pv = pvs->find ([=] (auto pv) { return (*pv)->id == id; })) {
		auto diffs = &(*pv.value ())->difficulties[difficulty];
		return diffs->find ([=] (auto diff) { return diff->isExtra == extra; });
	}
	return nullptr;
}

SprArgs::SprArgs () { DefaultSprArgs (this); }
} // namespace diva
