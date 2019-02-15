#define xFN(TYPE,NAME) TYPE ## _ ## NAME
#define FN(TYPE,NAME) xFN(TYPE,NAME)

/* Return an object that is equal to "obj" and that has only
 * a single reference.
 */
__isl_give TYPE *FN(TYPE,cow)(__isl_take TYPE *obj)
{
	TYPE *dup;

	if (!obj)
		return NULL;

	if (obj->ref == 1)
		return obj;
	dup = FN(TYPE,dup)(obj);
	FN(TYPE,free)(obj);
	return dup;
}
