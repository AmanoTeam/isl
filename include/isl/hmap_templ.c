/*
 * Copyright 2011      INRIA Saclay
 * Copyright 2013      Ecole Normale Superieure
 *
 * Use of this software is governed by the MIT license
 *
 * Written by Sven Verdoolaege, INRIA Saclay - Ile-de-France,
 * Parc Club Orsay Universite, ZAC des vignes, 4 rue Jacques Monod,
 * 91893 Orsay, France
 * and Ecole Normale Superieure, 45 rue d’Ulm, 75230 Paris, France
 */

#include <isl/ctx.h>
#include <isl/hash.h>
#include <isl/stream.h>

#define ISL_xCAT(A,B) A ## B
#define ISL_CAT(A,B) ISL_xCAT(A,B)
#define ISL_xFN(TYPE,NAME) TYPE ## _ ## NAME
#define ISL_FN(TYPE,NAME) ISL_xFN(TYPE,NAME)
#define ISL_xKV(TYPE1,TYPE2,NAME) isl_ ## TYPE1 ## _ ## TYPE2 ## _ ## NAME
#define ISL_yKV(TYPE1,TYPE2,NAME) ISL_xKV(TYPE1,TYPE2,NAME)
#define ISL_KV(NAME) ISL_yKV(ISL_KEY,ISL_VAL,NAME)
#define ISL_S(NAME) struct ISL_KV(NAME)

#define ISL_HMAP_EL		ISL_KV(pair)

#define ISL_HBASE		ISL_HMAP
#define ISL_HBASE_EL		ISL_HMAP_EL

struct ISL_HBASE {
	int ref;
	isl_ctx *ctx;
	struct isl_hash_table table;
};

struct ISL_HMAP_EL {
	ISL_KEY *key;
	ISL_VAL *val;
};
typedef struct ISL_HMAP_EL ISL_HMAP_EL;

__isl_give ISL_HBASE *ISL_FN(ISL_HBASE,alloc)(isl_ctx *ctx, int min_size)
{
	ISL_HBASE *hbase;

	hbase = isl_calloc_type(ctx, ISL_HBASE);
	if (!hbase)
		return NULL;

	hbase->ctx = ctx;
	isl_ctx_ref(ctx);
	hbase->ref = 1;

	if (isl_hash_table_init(ctx, &hbase->table, min_size) < 0)
		return ISL_FN(ISL_HBASE,free)(hbase);

	return hbase;
}

static void ISL_FN(ISL_HMAP_EL,free)(__isl_take ISL_HMAP_EL *pair)
{
	ISL_FN(ISL_KEY,free)(pair->key);
	ISL_FN(ISL_VAL,free)(pair->val);
	free(pair);
}

static isl_stat free_entry(void **entry, void *user)
{
	ISL_HBASE_EL *el = *entry;
	ISL_FN(ISL_HBASE_EL,free)(el);
	*entry = NULL;
	return isl_stat_ok;
}

__isl_null ISL_HBASE *ISL_FN(ISL_HBASE,free)(__isl_take ISL_HBASE *hbase)
{
	if (!hbase)
		return NULL;
	if (--hbase->ref > 0)
		return NULL;
	isl_hash_table_foreach(hbase->ctx, &hbase->table, &free_entry, NULL);
	isl_hash_table_clear(&hbase->table);
	isl_ctx_deref(hbase->ctx);
	free(hbase);
	return NULL;
}

isl_ctx *ISL_FN(ISL_HBASE,get_ctx)(__isl_keep ISL_HBASE *hbase)
{
	return hbase ? hbase->ctx : NULL;
}

/* Add a mapping from pair->key to pair->val to the associative array
 * pointed to by user.
 */
static isl_bool add_entry(void **entry, void *user)
{
	ISL_HMAP_EL *pair = *entry;
	ISL_HMAP **hmap = (ISL_HMAP **) user;
	ISL_KEY *key;
	ISL_VAL *val;

	key = ISL_FN(ISL_KEY,copy)(pair->key);
	val = ISL_FN(ISL_VAL,copy)(pair->val);
	*hmap = ISL_FN(ISL_HMAP,set)(*hmap, key, val);

	if (!*hmap)
		return isl_bool_error;

	return isl_bool_true;
}

/* Call "test" on every entry of "hbase".
 */
static isl_bool ISL_FN(ISL_HBASE,every_entry)(__isl_keep ISL_HBASE *hbase,
	isl_bool (*test)(void **entry, void *user), void *user)
{
	if (!hbase)
		return isl_bool_error;

	return isl_hash_table_every(hbase->ctx, &hbase->table, test, user);
}

__isl_give ISL_HBASE *ISL_FN(ISL_HBASE,dup)(__isl_keep ISL_HBASE *hbase)
{
	ISL_HBASE *dup;

	if (!hbase)
		return NULL;

	dup = ISL_FN(ISL_HBASE,alloc)(hbase->ctx, hbase->table.n);
	if (ISL_FN(ISL_HBASE,every_entry)(hbase, &add_entry, &dup) < 0)
		return ISL_FN(ISL_HBASE,free)(dup);

	return dup;
}

__isl_give ISL_HBASE *ISL_FN(ISL_HBASE,cow)(__isl_take ISL_HBASE *hbase)
{
	if (!hbase)
		return NULL;

	if (hbase->ref == 1)
		return hbase;
	hbase->ref--;
	return ISL_FN(ISL_HBASE,dup)(hbase);
}

__isl_give ISL_HBASE *ISL_FN(ISL_HBASE,copy)(__isl_keep ISL_HBASE *hbase)
{
	if (!hbase)
		return NULL;

	hbase->ref++;
	return hbase;
}

static isl_bool has_key(const void *entry, const void *c_key)
{
	const ISL_HMAP_EL *pair = entry;
	ISL_KEY *key = (ISL_KEY *) c_key;

	return ISL_KEY_IS_EQUAL(pair->key, key);
}

/* Does "pair" contain "val" as its value?
 */
static isl_bool has_val(ISL_HMAP_EL *pair, __isl_keep ISL_VAL *val)
{
	return ISL_VAL_IS_EQUAL(pair->val, val);
}

/* Free "key" and "val".
 */
static void free_key_val(__isl_take ISL_KEY *key, __isl_take ISL_VAL *val)
{
	ISL_FN(ISL_KEY,free)(key);
	ISL_FN(ISL_VAL,free)(val);
}

/* Replace the value of "pair" by "val".
 */
static void set_val(__isl_keep ISL_HMAP_EL *pair, __isl_take ISL_VAL *val)
{
	ISL_FN(ISL_VAL,free)(pair->val);
	pair->val = val;
}

/* Create a new entry from "key" and "val".
 */
static __isl_give ISL_HMAP_EL *create_entry(__isl_take ISL_KEY *key,
	__isl_take ISL_VAL *val)
{
	ISL_HMAP_EL *pair;

	pair = isl_alloc_type(ISL_FN(ISL_KEY,get_ctx)(key), ISL_HMAP_EL);
	if (!pair)
		goto error;

	pair->key = key;
	pair->val = val;
	return pair;
error:
	free_key_val(key, val);
	return NULL;
}

/* Optional "val" parameter declaration.
 */
#define OPT_VAL_PARAM		, __isl_take ISL_VAL *val
/* Optional "val" argument.
 */
#define OPT_VAL_ARG		, val

/* Add an entry with "key" and "val" (if specified) to "hhase".
 * If "key" was already mapped to something else, then that mapping
 * is replaced.
 * If key happened to be mapped to "val" already, then we leave
 * "hmap" untouched.
 */
static __isl_give ISL_HBASE *ISL_FN(ISL_HBASE,add)(__isl_take ISL_HBASE *hbase,
	__isl_take ISL_KEY *key OPT_VAL_PARAM)
{
	struct isl_hash_table_entry *entry;
	uint32_t hash;

	if (!hbase || !key)
		goto error;

	hash = ISL_FN(ISL_KEY,get_hash)(key);
	entry = isl_hash_table_find(hbase->ctx, &hbase->table, hash,
					&has_key, key, 0);
	if (!entry)
		goto error;
	if (entry != isl_hash_table_entry_none) {
		isl_bool equal;
		equal = has_val(entry->data OPT_VAL_ARG);
		if (equal < 0)
			goto error;
		if (equal) {
			free_key_val(key OPT_VAL_ARG);
			return hbase;
		}
	}

	hbase = ISL_FN(ISL_HBASE,cow)(hbase);
	if (!hbase)
		goto error;

	entry = isl_hash_table_find(hbase->ctx, &hbase->table, hash,
					&has_key, key, 1);

	if (!entry)
		goto error;

	if (entry->data) {
		set_val(entry->data OPT_VAL_ARG);
		ISL_FN(ISL_KEY,free)(key);
		return hbase;
	}

	entry->data = create_entry(key OPT_VAL_ARG);
	if (!entry->data)
		return ISL_FN(ISL_HBASE,free)(hbase);

	return hbase;
error:
	free_key_val(key OPT_VAL_ARG);
	return ISL_FN(ISL_HBASE,free)(hbase);
}

/* If "hmap" contains a value associated to "key", then return
 * (isl_bool_true, copy of value).
 * Otherwise, return
 * (isl_bool_false, NULL).
 * If an error occurs, then return
 * (isl_bool_error, NULL).
 */
__isl_give ISL_MAYBE(ISL_VAL) ISL_FN(ISL_HMAP,try_get)(
	__isl_keep ISL_HMAP *hmap, __isl_keep ISL_KEY *key)
{
	struct isl_hash_table_entry *entry;
	ISL_HMAP_EL *pair;
	uint32_t hash;
	ISL_MAYBE(ISL_VAL) res = { isl_bool_false, NULL };

	if (!hmap || !key)
		goto error;

	hash = ISL_FN(ISL_KEY,get_hash)(key);
	entry = isl_hash_table_find(hmap->ctx, &hmap->table, hash,
					&has_key, key, 0);

	if (!entry)
		goto error;
	if (entry == isl_hash_table_entry_none)
		return res;

	pair = entry->data;

	res.valid = isl_bool_true;
	res.value = ISL_FN(ISL_VAL,copy)(pair->val);
	if (!res.value)
		res.valid = isl_bool_error;
	return res;
error:
	res.valid = isl_bool_error;
	res.value = NULL;
	return res;
}

/* If "hmap" contains a value associated to "key", then return
 * isl_bool_true.  Otherwise, return isl_bool_false.
 * Return isl_bool_error on error.
 */
isl_bool ISL_FN(ISL_HMAP,has)(__isl_keep ISL_HMAP *hmap,
	__isl_keep ISL_KEY *key)
{
	ISL_MAYBE(ISL_VAL) res;

	res = ISL_FN(ISL_HMAP,try_get)(hmap, key);
	ISL_FN(ISL_VAL,free)(res.value);

	return res.valid;
}

/* If "hmap" contains a value associated to "key", then return
 * a copy of that value.  Otherwise, return NULL.
 * Return NULL on error.
 */
__isl_give ISL_VAL *ISL_FN(ISL_HMAP,get)(__isl_keep ISL_HMAP *hmap,
	__isl_take ISL_KEY *key)
{
	ISL_VAL *res;

	res = ISL_FN(ISL_HMAP,try_get)(hmap, key).value;
	ISL_FN(ISL_KEY,free)(key);
	return res;
}

/* Remove the mapping between "key" and its associated value (if any)
 * from "hmap".
 *
 * If "key" is not mapped to anything, then we leave "hmap" untouched"
 */
__isl_give ISL_HMAP *ISL_FN(ISL_HMAP,drop)(__isl_take ISL_HMAP *hmap,
	__isl_take ISL_KEY *key)
{
	struct isl_hash_table_entry *entry;
	ISL_HMAP_EL *pair;
	uint32_t hash;

	if (!hmap || !key)
		goto error;

	hash = ISL_FN(ISL_KEY,get_hash)(key);
	entry = isl_hash_table_find(hmap->ctx, &hmap->table, hash,
					&has_key, key, 0);
	if (!entry)
		goto error;
	if (entry == isl_hash_table_entry_none) {
		ISL_FN(ISL_KEY,free)(key);
		return hmap;
	}

	hmap = ISL_FN(ISL_HMAP,cow)(hmap);
	if (!hmap)
		goto error;
	entry = isl_hash_table_find(hmap->ctx, &hmap->table, hash,
					&has_key, key, 0);
	ISL_FN(ISL_KEY,free)(key);

	if (!entry)
		return ISL_FN(ISL_HMAP,free)(hmap);
	if (entry == isl_hash_table_entry_none)
		isl_die(hmap->ctx, isl_error_internal,
			"missing entry" , return ISL_FN(ISL_HMAP,free)(hmap));

	pair = entry->data;
	isl_hash_table_remove(hmap->ctx, &hmap->table, entry);
	ISL_FN(ISL_KEY,free)(pair->key);
	ISL_FN(ISL_VAL,free)(pair->val);
	free(pair);

	return hmap;
error:
	ISL_FN(ISL_KEY,free)(key);
	ISL_FN(ISL_HMAP,free)(hmap);
	return NULL;
}

/* Add a mapping from "key" to "val" to "hmap".
 */
__isl_give ISL_HMAP *ISL_FN(ISL_HMAP,set)(__isl_take ISL_HMAP *hmap,
	__isl_take ISL_KEY *key, __isl_take ISL_VAL *val)
{
	if (!val)
		key = ISL_FN(ISL_KEY,free)(key);
	return ISL_FN(ISL_HMAP,add)(hmap, key, val);
}

/* Internal data structure for isl_*_to_*_foreach.
 *
 * fn is the function that should be called on each entry.
 * user is the user-specified final argument to fn.
 */
ISL_S(foreach_data) {
	isl_stat (*fn)(__isl_take ISL_KEY *key, __isl_take ISL_VAL *val,
		void *user);
	void *user;
};

/* Call data->fn on a copy of the key and value in *entry.
 */
static isl_stat call_on_copy(void **entry, void *user)
{
	ISL_HMAP_EL *pair = *entry;
	ISL_S(foreach_data) *data = (ISL_S(foreach_data) *) user;

	return data->fn(ISL_FN(ISL_KEY,copy)(pair->key),
			ISL_FN(ISL_VAL,copy)(pair->val), data->user);
}

/* Call "fn" on each pair of key and value in "hmap".
 */
isl_stat ISL_FN(ISL_HMAP,foreach)(__isl_keep ISL_HMAP *hmap,
	isl_stat (*fn)(__isl_take ISL_KEY *key, __isl_take ISL_VAL *val,
		void *user),
	void *user)
{
	ISL_S(foreach_data) data = { fn, user };

	if (!hmap)
		return isl_stat_error;

	return isl_hash_table_foreach(hmap->ctx, &hmap->table,
				      &call_on_copy, &data);
}

/* Internal data structure for isl_*_to_*_every.
 *
 * "test" is the function that should be called on each entry,
 * until any invocation returns isl_bool_false.
 * "test_user" is the user-specified final argument to "test".
 */
ISL_S(every_data) {
	isl_bool (*test)(__isl_keep ISL_KEY *key, __isl_keep ISL_VAL *val,
		void *user);
	void *test_user;
};

/* Call data->test on the key and value in *entry.
 */
static isl_bool call_on_pair(void **entry, void *user)
{
	ISL_HMAP_EL *pair = *entry;
	ISL_S(every_data) *data = (ISL_S(every_data) *) user;

	return data->test(pair->key, pair->val, data->test_user);
}

/* Does "test" succeed on every entry of "hmap"?
 */
isl_bool ISL_FN(ISL_HMAP,every)(__isl_keep ISL_HMAP *hmap,
	isl_bool (*test)(__isl_keep ISL_KEY *key, __isl_keep ISL_VAL *val,
		void *user),
	void *user)
{
	ISL_S(every_data) data = { test, user };

	if (!hmap)
		return isl_bool_error;

	return isl_hash_table_every(hmap->ctx, &hmap->table,
				      &call_on_pair, &data);
}

#ifdef ISL_HMAP_IS_EQUAL

/* Does "hmap" have an entry with key "key" and value "val"?
 */
static isl_bool has_entry(__isl_keep ISL_KEY *key, __isl_keep ISL_VAL *val,
	void *user)
{
	ISL_HMAP *hmap = user;
	ISL_MAYBE(ISL_VAL) maybe_val;
	isl_bool equal;

	maybe_val = ISL_FN(ISL_HMAP,try_get)(hmap, key);
	if (maybe_val.valid < 0 || !maybe_val.valid)
		return maybe_val.valid;
	equal = ISL_VAL_IS_EQUAL(maybe_val.value, val);
	ISL_FN(ISL_VAL,free)(maybe_val.value);
	return equal;
}

/* Is "hmap1" (obviously) equal to "hmap2"?
 *
 * In particular, do the two associative arrays have
 * the same number of entries and does every entry of the first
 * also appear in the second?
 */
isl_bool ISL_HMAP_IS_EQUAL(__isl_keep ISL_HMAP *hmap1,
	__isl_keep ISL_HMAP *hmap2)
{
	if (!hmap1 || !hmap2)
		return isl_bool_error;
	if (hmap1 == hmap2)
		return isl_bool_true;
	if (hmap1->table.n != hmap2->table.n)
		return isl_bool_false;
	return ISL_FN(ISL_HMAP,every)(hmap1, &has_entry, hmap2);
}

#endif

/* Internal data structure for print_pair.
 *
 * p is the printer on which the associative array is being printed.
 * first is set if the current key-value pair is the first to be printed.
 */
ISL_S(print_data) {
	isl_printer *p;
	int first;
};

/* Print the given key-value pair to data->p.
 */
static isl_stat print_pair(__isl_take ISL_KEY *key, __isl_take ISL_VAL *val,
	void *user)
{
	ISL_S(print_data) *data = user;

	if (!data->first)
		data->p = isl_printer_print_str(data->p, ", ");
	data->p = ISL_KEY_PRINT(data->p, key);
	data->p = isl_printer_print_str(data->p, ": ");
	data->p = ISL_VAL_PRINT(data->p, val);
	data->first = 0;

	ISL_FN(ISL_KEY,free)(key);
	ISL_FN(ISL_VAL,free)(val);
	return isl_stat_ok;
}

/* Print the associative array to "p".
 */
__isl_give isl_printer *ISL_FN(isl_printer_print,ISL_HMAP_SUFFIX)(
	__isl_take isl_printer *p, __isl_keep ISL_HMAP *hmap)
{
	ISL_S(print_data) data;

	if (!p || !hmap)
		return isl_printer_free(p);

	p = isl_printer_print_str(p, "{");
	data.p = p;
	data.first = 1;
	if (ISL_FN(ISL_HMAP,foreach)(hmap, &print_pair, &data) < 0)
		data.p = isl_printer_free(data.p);
	p = data.p;
	p = isl_printer_print_str(p, "}");

	return p;
}

void ISL_FN(ISL_HMAP,dump)(__isl_keep ISL_HMAP *hmap)
{
	isl_printer *printer;

	if (!hmap)
		return;

	printer = isl_printer_to_file(ISL_FN(ISL_HMAP,get_ctx)(hmap), stderr);
	printer = ISL_FN(isl_printer_print,ISL_HMAP_SUFFIX)(printer, hmap);
	printer = isl_printer_end_line(printer);

	isl_printer_free(printer);
}

/* Return a string representation of "hmap".
 */
__isl_give char *ISL_FN(ISL_HMAP,to_str)(__isl_keep ISL_HMAP *hmap)
{
	isl_printer *p;
	char *s;

	if (!hmap)
		return NULL;
	p = isl_printer_to_str(ISL_FN(ISL_HMAP,get_ctx)(hmap));
	p = ISL_FN(isl_printer_print,ISL_HMAP_SUFFIX)(p, hmap);
	s = isl_printer_get_str(p);
	isl_printer_free(p);

	return s;
}

#ifdef ISL_HMAP_HAVE_READ_FROM_STR

/* Read an associative array from "s".
 * The input format corresponds to the way associative arrays are printed
 * by isl_printer_print_*_to_*.
 * In particular, each key-value pair is separated by a colon,
 * the key-value pairs are separated by a comma and
 * the entire associative array is surrounded by braces.
 */
__isl_give ISL_HMAP *ISL_FN(isl_stream_read,ISL_HMAP_SUFFIX)(isl_stream *s)
{
	isl_ctx *ctx;
	ISL_HMAP *hmap;

	if (!s)
		return NULL;
	ctx = isl_stream_get_ctx(s);
	hmap = ISL_FN(ISL_HMAP,alloc)(ctx, 0);
	if (!hmap)
		return NULL;
	if (isl_stream_eat(s, '{') < 0)
		return ISL_FN(ISL_HMAP,free)(hmap);
	if (isl_stream_eat_if_available(s, '}'))
		return hmap;
	do {
		ISL_KEY *key;
		ISL_VAL *val = NULL;

		key = ISL_KEY_READ(s);
		if (isl_stream_eat(s, ':') >= 0)
			val = ISL_VAL_READ(s);
		hmap = ISL_FN(ISL_HMAP,set)(hmap, key, val);
		if (!hmap)
			return NULL;
	} while (isl_stream_eat_if_available(s, ','));
	if (isl_stream_eat(s, '}') < 0)
		return ISL_FN(ISL_HMAP,free)(hmap);
	return hmap;
}

/* Read an associative array from the string "str".
 * The input format corresponds to the way associative arrays are printed
 * by isl_printer_print_*_to_*.
 * In particular, each key-value pair is separated by a colon,
 * the key-value pairs are separated by a comma and
 * the entire associative array is surrounded by braces.
 */
__isl_give ISL_HMAP *ISL_FN(ISL_HMAP,read_from_str)(isl_ctx *ctx,
	const char *str)
{
	ISL_HMAP *hmap;
	isl_stream *s;

	s = isl_stream_new_str(ctx, str);
	if (!s)
		return NULL;
	hmap = ISL_FN(isl_stream_read,ISL_HMAP_SUFFIX)(s);
	isl_stream_free(s);
	return hmap;
}

#endif
