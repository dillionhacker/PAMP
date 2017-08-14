
#ifndef S60_EXT_TOOLS_H_
#define S60_EXT_TOOLS_H_

extern "C" 
{
#include "php.h"
}

#include <e32std.h>


/**
 * Use this S60_EMULATOR define to make different compiles
 * for emulator.
 */
#ifdef __WINS__
#define S60_EMULATOR
#endif


/************************************************
 * Class creation helpers
 * These should not generally be used, but the
 * real helpers below these.
 ************************************************/
#define FREE_INTERN(_intern_, _object_) \
	{ zend_object_std_dtor(&_intern_->std TSRMLS_CC); \
	efree(_object_); }

#define INITIALIZE_INTERN(_intern_, _intern_type_) \
	{ zval *tmp; \
	_intern_ = (_intern_type_*)emalloc(sizeof(_intern_type_)); \
	memset(_intern_, 0, sizeof(_intern_type_)); \
	zend_object_std_init(&_intern_->std, class_type TSRMLS_CC); \
	zend_hash_copy(_intern_->std.properties, &class_type->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *)); }
	
#define REGISTER_BASIC_CLASS(_class_entry_, _class_name_, _class_funcs_, _class_new_fn_, _class_global_ce_, _class_object_handlers_) \
	{ \
	TSRMLS_FETCH(); \
	INIT_CLASS_ENTRY(_class_entry_, _class_name_, _class_funcs_);\
	_class_entry_.create_object = _class_new_fn_;\
	_class_global_ce_ = zend_register_internal_class_ex(&_class_entry_, NULL, NULL TSRMLS_CC);\
	memcpy(&_class_object_handlers_, zend_get_std_object_handlers(), sizeof(zend_object_handlers));\
	_class_object_handlers_.clone_obj = NULL; }
	
#define INITIALIZE_OBJECT_VALUE(_ov_, _intern_, _free_fn_, _object_handlers_) \
	{ _ov_.handle = zend_objects_store_put(_intern_, (zend_objects_store_dtor_t)zend_objects_destroy_object, (zend_objects_free_object_storage_t) _free_fn_, NULL TSRMLS_CC); \
	_ov_.handlers = &_object_handlers_; }

/************************************************
 * More class creation helpers
 * See s60_contacts for usage examples
 ************************************************/
 
// Type of struct associated with PHP class
#define CLASS_STRUCT(classname) classname##_obj
// Name of PHP class entry global variable,
// as defined by DEFINE_CLASS_DATA
#define CLASS_ENTRY(classname) classname##_ce

#define BEGIN_CLASS_STRUCT(classname) \
typedef struct _##classname##_obj classname##_obj; \
struct _##classname##_obj { \
	zend_object std;

#define END_CLASS_STRUCT() };

#define DEFINE_CLASS_DATA(classname) \
	zend_class_entry *classname##_ce; \
	static zend_object_handlers classname##_object_handlers;

#define BEGIN_CLASS_FUNCTIONS(classname) \
	zend_function_entry classname##_funcs[] = {

#define END_CLASS_FUNCTIONS() \
	{NULL, NULL, NULL} };

#define DEFINE_OBJECT_FREE_FUNCTION(classname, _code) \
	static void classname##_object_free_storage(void *object TSRMLS_DC) \
	{ classname##_obj *self = (classname##_obj *)object; \
	_code; \
	FREE_INTERN(self, object) }

#define DEFINE_OBJECT_NEW_FUNCTION(classname, _code) \
	static zend_object_value classname##_object_new(zend_class_entry *class_type TSRMLS_DC) { \
		classname##_obj *self; \
		zend_object_value retval; \
		INITIALIZE_INTERN(self, classname##_obj) \
		INITIALIZE_OBJECT_VALUE(retval, self, classname##_object_free_storage, classname##_object_handlers) \
		_code; \
		return retval; }

#define CLASS_REGISTER_FUNCTION(classname) static void register_##classname##_class(void)
#define CLASS_REGISTER_CALL(classname) register_##classname##_class()
#define DEFINE_CLASS_REGISTER_FUNCTION(classname) \
	static void register_##classname##_class(void) \
	{ \
		zend_class_entry ce; \
		REGISTER_BASIC_CLASS(ce, #classname, classname##_funcs, classname##_object_new, classname##_ce, classname##_object_handlers); \
	}

/************************************************
 * Method and function writing helpers
 * See s60_contacts for usage examples
 ************************************************/	
 
// FUNCTION ARGUMENT GETTERS
#define GET_FUNCTION_ARGS_0() \
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) { \
		RETURN_NULL(); \
	}
	
#define GET_FUNCTION_ARGS_1(format, data1) \
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, format, data1) == FAILURE) { \
		RETURN_NULL(); \
	}
	
#define GET_FUNCTION_ARGS_2(format, data1, data2) \
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, format, data1, data2) == FAILURE) { \
		RETURN_NULL(); \
	}
	
#define GET_FUNCTION_ARGS_3(format, data1, data2, data3) \
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, format, data1, data2, data3) == FAILURE) { \
		RETURN_NULL(); \
	}
		
#define GET_FUNCTION_ARGS_4(format, data1, data2, data3, data4) \
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, format, data1, data2, data3, data4) == FAILURE) { \
		RETURN_NULL(); \
	}
	
// MACRO FOR GETTING INTERN OBJECT FROM OBJECT STORE
#define GET_STRUCT(_self_zv_, classname) ((CLASS_STRUCT(classname)*)zend_object_store_get_object(_self_zv_ TSRMLS_CC))

// METHOD ARGUMENT GETTERS
#define GET_METHOD_ARGS_0(classname) \
	CLASS_STRUCT(classname) *self; zval *self_zv; \
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &self_zv, CLASS_ENTRY(classname)) == FAILURE) { \
		RETURN_NULL(); \
	} \
	self = GET_STRUCT(self_zv, classname);
	
#define GET_METHOD_ARGS_1(classname, format, data1) \
	CLASS_STRUCT(classname) *self; zval *self_zv; \
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), format, &self_zv, CLASS_ENTRY(classname), data1) == FAILURE) { \
		RETURN_NULL(); \
	} \
	self = GET_STRUCT(self_zv, classname);
	
#define GET_METHOD_ARGS_2(classname, format, data1, data2) \
	CLASS_STRUCT(classname) *self; zval *self_zv; \
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), format, &self_zv, CLASS_ENTRY(classname), data1, data2) == FAILURE) { \
		RETURN_NULL(); \
	} \
	self = GET_STRUCT(self_zv, classname);
	
#define GET_METHOD_ARGS_3(classname, format, data1, data2, data3) \
	CLASS_STRUCT(classname) *self; zval *self_zv; \
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), format, &self_zv, CLASS_ENTRY(classname), data1, data2, data3) == FAILURE) { \
		RETURN_NULL(); \
	} \
	self = GET_STRUCT(self_zv, classname);
	
#define GET_METHOD_ARGS_4(classname, format, data1, data2, data3, data4) \
	CLASS_STRUCT(classname) *self; zval *self_zv; \
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), format, &self_zv, CLASS_ENTRY(classname), data1, data2, data3, data4) == FAILURE) { \
		RETURN_NULL(); \
	} \
	self = GET_STRUCT(self_zv, classname);
	
#define GET_METHOD_ARGS_5(classname, format, data1, data2, data3, data4, data5) \
	CLASS_STRUCT(classname) *self; zval *self_zv; \
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), format, &self_zv, CLASS_ENTRY(classname), data1, data2, data3, data4, data5) == FAILURE) { \
		RETURN_NULL(); \
	} \
	self = GET_STRUCT(self_zv, classname);

// METHOD ARGUMENT FETCHERS (allows varargs)
#define FETCH_METHOD_ARGS_0(classname) \
	CLASS_STRUCT(classname) *self; zval *self_zv = getThis(); \
	if (Z_TYPE_P(self_zv) != IS_OBJECT || \
			(!instanceof_function(Z_OBJCE_P(self_zv), (CLASS_ENTRY(classname)) TSRMLS_CC))) { \
		zend_error(E_WARNING, "%s::%s() must be derived from %s::%s", \
			(CLASS_ENTRY(classname))->name, get_active_function_name(TSRMLS_C), Z_OBJCE_P(self_zv)->name, get_active_function_name(TSRMLS_C)); \
		RETURN_NULL(); \
	} \
	self = GET_STRUCT(self_zv, classname);
	
	
/************************************************
 * Module creation helpers
 * See s60_contacts for usage examples
 ************************************************/
#define BEGIN_MODULE_FUNCTIONS(modulename) \
	static function_entry modulename##_functions[] = {

#define END_MODULE_FUNCTIONS() \
	{NULL, NULL, NULL} };
	
#define DEFINE_MODULE(modulename, moduleversion, minit, mfree, rinit, rfree) \
	zend_module_entry modulename##_module_entry = { \
		STANDARD_MODULE_HEADER, \
		#modulename, \
		modulename##_functions, \
		minit, \
		mfree, \
		rinit, \
		rfree, \
		NULL, \
		moduleversion, \
		STANDARD_MODULE_PROPERTIES \
	};

/************************************************
 * Error value returners
 ************************************************/
 

const static int KErrPhpCustom = -22142; 

#define LEAVE_ERROR(_err_text_) { zend_error(E_WARNING, _err_text_); User::Leave(KErrPhpCustom); } 
#define RETURN_ERROR(_err_text_) { zend_error(E_WARNING, _err_text_); zval_dtor(return_value); RETURN_NULL(); } 
#define RETURN_SYMBIAN_ERROR(__ERr_) { set_symbian_error(__ERr_); zval_dtor(return_value); RETURN_NULL(); }
#define RETURN_IF_SYMBIAN_ERROR(__ERr_) if ((__ERr_) != KErrNone) { RETURN_SYMBIAN_ERROR(__ERr_); }
#define RETURN_NOTIMPLEMENTED_ERROR() { RETURN_ERROR("Feature not implemented"); }

#ifdef __cplusplus
extern "C" 
{
#endif

/** 
 * Sets zend_error corresponding to given symbian error code
 */
void set_symbian_error(TInt error);

/************************************************
 * Conversion functions
 ************************************************/
 
/**
 * Assigns given input string to zval output, using UTF-8 encoding.
 */
TInt ZVAL_SYMBIAN_STRING(zval* output, const TDesC& input);

/**
 * Converts path (e.g. "foo") to real path (e.g. "c:\\bar\\foo")
 */
TInt convert_path_to_realpath(char* resolved_path, const char* path);

/**
 * First calls convert_path_to_realpath, then convert_c_string_to_unicode
 */
TInt convert_c_path_to_unicode_realpath(TDes& uc, const char* string);

/**
 * Converts C string (assuming UTF-8) to TDes unicode string. Returns symbian error code.
 */
TInt convert_c_string_to_unicode(TDes& uc, const char* string, int string_len);

/**
 * Converts C string (assuming UTF-8) to HBufC*, HBufC will be in cleanup stack.
 */
HBufC* convert_c_string_to_unicodeLC(const char* string, int string_len);

/**
 * Converts local TTime to utc TReal
 */
TReal convert_time_to_utc_real(const TTime& aTime);

/**
 * Converts utc TReal to local TTime
 */
TTime convert_utc_real_to_time(TReal unixtime);

/**
 * Converts utc TTime to utc TReal
 */
TReal convert_utc_time_to_utc_real(const TTime& aTime);

/**
 * Converts utc TReal to utc TTime
 */
TTime convert_utc_real_to_utc_time(TReal unixtime);

#ifdef __cplusplus
}
#endif

extern "C" double php_e32_clock();
extern "C" double php_e32_UTC_offset();

inline double Z_DVAL_SAFE(zval value) {
	double ret;
	zval_copy_ctor(&value);
	convert_to_double(&value);
	ret = Z_DVAL(value);
	zval_dtor(&value);
	return ret;
}

#define ARRAY_FOREACH(pointer_, arr_hash_, data_) \
	HashPosition pointer_; \
	for (zend_hash_internal_pointer_reset_ex(arr_hash_, &pointer_); \
	     zend_hash_get_current_data_ex(arr_hash_, (void**) &data_, &pointer_) == SUCCESS; \
	     zend_hash_move_forward_ex(arr_hash_, &pointer_))

/************************************************
 * Miscellaneous
 ************************************************/
 
#define IS_RETVAL_TRUE() (Z_TYPE_P(return_value) == IS_BOOL && Z_LVAL_P(return_value))

/*
 * Tests if the file exists.
 */
TBool doesFileExistL(TDesC& filename);

/** 
 * Abstract base class for reference counted resources.
 */
class Refcounted {
	long refcount;
protected:
	virtual ~Refcounted() {}; // protected destructor; Use decRef()
public:

	Refcounted() : refcount(1) {}
	
	void incRef() {
		refcount++;
	}

	void decRef() {
		refcount--;
		if (refcount == 0) {
			delete this; // cleans up itself
		}
	}
};

/** To be used with pointers to Refcounted objects. */
#define SAFE_CLEAR_REFERENCE(_var_) \
	{ if (_var_) _var_->decRef(); \
	_var_ = 0; }



#endif
