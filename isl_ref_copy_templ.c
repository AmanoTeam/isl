#define xFN(TYPE,NAME) TYPE ## _ ## NAME
#define FN(TYPE,NAME) xFN(TYPE,NAME)

/* Return a new reference to "obj".
 */
__isl_give TYPE *FN(TYPE,copy)(__isl_keep TYPE *obj)
{
	if (!obj)
		return NULL;

	obj->ref++;
	return obj;
}
