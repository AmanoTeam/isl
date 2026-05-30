#include <isl/ctx.h>
#include <isl/maybe.h>
#include <isl/printer.h>

#define ISL_HBASE			ISL_HMAP
#define ISL_HBASE_SUFFIX		ISL_HMAP_SUFFIX
#ifdef ISL_HMAP_IS_EQUAL
#define ISL_HBASE_IS_EQUAL		ISL_HMAP_IS_EQUAL
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#define ISL_xCAT(A,B) A ## B
#define ISL_CAT(A,B) ISL_xCAT(A,B)
#define ISL_xFN(TYPE,NAME) TYPE ## _ ## NAME
#define ISL_FN(TYPE,NAME) ISL_xFN(TYPE,NAME)

struct __isl_export ISL_HBASE;
typedef struct ISL_HBASE	ISL_HBASE;

__isl_constructor
__isl_give ISL_HBASE *ISL_FN(ISL_HBASE,alloc)(isl_ctx *ctx, int min_size);
__isl_give ISL_HBASE *ISL_FN(ISL_HBASE,copy)(__isl_keep ISL_HBASE *hbase);
__isl_null ISL_HBASE *ISL_FN(ISL_HBASE,free)(__isl_take ISL_HBASE *hbase);

isl_ctx *ISL_FN(ISL_HBASE,get_ctx)(__isl_keep ISL_HBASE *hbase);

__isl_give ISL_MAYBE(ISL_VAL) ISL_FN(ISL_HMAP,try_get)(
	__isl_keep ISL_HMAP *hmap, __isl_keep ISL_KEY *key);
isl_bool ISL_FN(ISL_HMAP,has)(__isl_keep ISL_HMAP *hmap,
	__isl_keep ISL_KEY *key);
__isl_give ISL_VAL *ISL_FN(ISL_HMAP,get)(__isl_keep ISL_HMAP *hmap,
	__isl_take ISL_KEY *key);
__isl_export
__isl_give ISL_HMAP *ISL_FN(ISL_HMAP,set)(__isl_take ISL_HMAP *hmap,
	__isl_take ISL_KEY *key, __isl_take ISL_VAL *val);
__isl_give ISL_HBASE *ISL_FN(ISL_HBASE,drop)(__isl_take ISL_HBASE *hbase,
	__isl_take ISL_KEY *key);

isl_stat ISL_FN(ISL_HMAP,foreach)(__isl_keep ISL_HMAP *hmap,
	isl_stat (*fn)(__isl_take ISL_KEY *key, __isl_take ISL_VAL *val,
		void *user),
	void *user);
isl_bool ISL_FN(ISL_HMAP,every)(__isl_keep ISL_HMAP *hmap,
	isl_bool (*test)(__isl_keep ISL_KEY *key, __isl_keep ISL_VAL *val,
		void *user),
	void *user);

#ifdef ISL_HBASE_IS_EQUAL
__isl_export
isl_bool ISL_HBASE_IS_EQUAL(__isl_keep ISL_HBASE *hbase1,
	__isl_keep ISL_HBASE *hbase2);
#endif

#ifdef ISL_HMAP_HAVE_READ_FROM_STR
__isl_constructor
__isl_give ISL_HMAP *ISL_FN(ISL_HMAP,read_from_str)(isl_ctx *ctx,
	const char *str);
#endif
__isl_give char *ISL_FN(ISL_HMAP,to_str)(__isl_keep ISL_HMAP *hmap);
__isl_give isl_printer *ISL_FN(isl_printer_print,ISL_HBASE_SUFFIX)(
	__isl_take isl_printer *p, __isl_keep ISL_HBASE *hbase);
void ISL_FN(ISL_HMAP,dump)(__isl_keep ISL_HMAP *hmap);

#undef ISL_xCAT
#undef ISL_CAT
#undef ISL_KEY
#undef ISL_VAL
#undef ISL_xFN
#undef ISL_FN
#undef ISL_xHMAP
#undef ISL_yHMAP
#undef ISL_HMAP

#if defined(__cplusplus)
}
#endif

#undef ISL_HBASE
#undef ISL_HBASE_SUFFIX
#undef ISL_HBASE_IS_EQUAL
