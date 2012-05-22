/*
 *
 * @file dt.c
 *
 * @brief Aggregate and utility functions written in C for C45 and RF in MADlib
 *
 * @date April 10, 2012
 */

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "postgres.h"
#include "fmgr.h"
#include "utils/array.h"
#include "utils/lsyscache.h"
#include "utils/builtins.h"
#include "catalog/pg_type.h"
#include "catalog/namespace.h"
#include "nodes/execnodes.h"
#include "nodes/nodes.h"
#include "funcapi.h"

#ifndef NO_PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif
/*#define __DT_SHOW_DEBUG_INFO__*/
#ifdef __DT_SHOW_DEBUG_INFO__
#define dtelog(...) elog(__VA_ARGS__)
#else
#define dtelog(...)
#endif


/*
 * Postgres8.4 doesn't have such macro, so we add here
 */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
#endif


/*
 * This function is used to get the mask of the given value
 * val - ((val >> power) << power) equals to val % (2^power)
 */
#define dt_fid_mask(val, power) \
			(1 << (val - ((val >> power) << power)))


/*
 * This function is used to test if a float value is 0.
 * Due to the precision of floating numbers, we can not
 * compare them directly with 0.
 */
#define dt_is_float_zero(value)  \
			((value) < 1E-10 && (value) > -1E-10)


#define dt_check_error_value(condition, message, value) \
			do { \
				if (!(condition)) \
					ereport(ERROR, \
							(errcode(ERRCODE_RAISE_EXCEPTION), \
							 errmsg(message, (value)) \
							) \
						   ); \
			} while (0)


#define dt_check_error(condition, message) \
			do { \
				if (!(condition)) \
					ereport(ERROR, \
							(errcode(ERRCODE_RAISE_EXCEPTION), \
							 errmsg(message) \
							) \
						   ); \
			} while (0)



Datum
condition_set
    (
    PG_FUNCTION_ARGS
    )
{
	bool condition = PG_GETARG_BOOL(0);
	float8 true_value = PG_GETARG_FLOAT8(1);
	float8 false_value = PG_GETARG_FLOAT8(2);
	float8 result = 0.0;

	if (condition)
		result = true_value;
	else
		result = false_value;

	PG_RETURN_FLOAT8(result);
}
PG_FUNCTION_INFO_V1(condition_set);


Datum
equal_set
    (
    PG_FUNCTION_ARGS
    )
{
	float8 real_value = PG_GETARG_FLOAT8(0);
	float8 con_value = PG_GETARG_FLOAT8(1);
	float8 set_value = PG_GETARG_FLOAT8(2);
	float8 result = 0.0;

	if (PG_ARGISNULL(0))
		result = set_value;
	else if (dt_is_float_zero(real_value - con_value))
		result = set_value;
	else if(!dt_is_float_zero(real_value - con_value))
		result = real_value;

	PG_RETURN_FLOAT8(result);
}
PG_FUNCTION_INFO_V1(equal_set);

Datum
less_set
    (
    PG_FUNCTION_ARGS
    )
{

	float8 real_value = PG_GETARG_FLOAT8(0);
	float8 con_value = PG_GETARG_FLOAT8(1);
	float8 set_value = PG_GETARG_FLOAT8(2);
	float8 result = 0.0;

	if (PG_ARGISNULL(0))
		result = set_value;
	else if (real_value < con_value)
		result = set_value;
	else if(real_value > con_value)
		result = real_value;

	PG_RETURN_FLOAT8(result);
}
PG_FUNCTION_INFO_V1(less_set);

Datum
greater_set
    (
    PG_FUNCTION_ARGS
    )
{

	float8 real_value = PG_GETARG_FLOAT8(0);
	float8 con_value = PG_GETARG_FLOAT8(1);
	float8 set_value = PG_GETARG_FLOAT8(2);
	float8 result = 0.0;

	if (PG_ARGISNULL(0))
		result = set_value;
	else if (real_value > con_value)
		result = set_value;
	else if(real_value < con_value)
		result = real_value;

	PG_RETURN_FLOAT8(result);
}
PG_FUNCTION_INFO_V1(greater_set);

Datum
is_float8_zero
    (
    PG_FUNCTION_ARGS
    )
{
	float8 real_value = PG_GETARG_FLOAT8(0);
	bool	result = 1;

	if (PG_ARGISNULL(0))
		result = 0;
	else
		result = dt_is_float_zero(real_value);

	PG_RETURN_BOOL(result);
}
PG_FUNCTION_INFO_V1(is_float8_zero);


Datum
get_category_sim
    (
    PG_FUNCTION_ARGS
    )
{
    ArrayType *cat1_array    = PG_GETARG_ARRAYTYPE_P(0);
	ArrayType *cat2_array    = PG_GETARG_ARRAYTYPE_P(1);

    int array_dim               = 0;
    int *p_array_dim            = NULL;
    int array_length            = 0;
    float8 *cat1_data			= NULL;
	float8 *cat2_data			= NULL;
	float8 sim					= 1.0;

    array_dim			= ARR_NDIM(cat1_array);
    p_array_dim         = ARR_DIMS(cat1_array);
    array_length        = ArrayGetNItems(array_dim,p_array_dim);
    cat1_data           = (float8 *)ARR_DATA_PTR(cat1_array);

    array_dim			= ARR_NDIM(cat2_array);
    p_array_dim         = ARR_DIMS(cat2_array);
    array_length        = ArrayGetNItems(array_dim,p_array_dim);
    cat2_data           = (float8 *)ARR_DATA_PTR(cat2_array);
	
	//elog(NOTICE, "%lf, %lf, %lf, %lf", cat1_data[0], cat1_data[1], cat1_data[2], cat1_data[3]);
	//elog(NOTICE, "%lf, %lf, %lf, %lf", cat2_data[0], cat2_data[1], cat2_data[2], cat2_data[3]);
	
	if (!dt_is_float_zero(cat1_data[0] - cat2_data[0]))
		sim = 0;
	else if (!dt_is_float_zero(cat1_data[1] - cat2_data[1]))
		sim = (float8)1.0/15.0;
	else if (!dt_is_float_zero(cat1_data[2] - cat2_data[2]))
		sim = (float8)3.0/15.0;
	else if (!dt_is_float_zero(cat1_data[3] - cat2_data[3]))
		sim =  (float8)8.0/15.0;
	else 
		sim = 1.0;

	PG_RETURN_FLOAT8((float8)sim);
}
PG_FUNCTION_INFO_V1(get_category_sim);


/*
 * @brief Use % as the delimiter to split the given string. The char '\' is used
 *        to escape %. We will not change the default behavior of '\' in PG/GP.
 *        For example, assume the given string is E"\\\\\\\\\\%123%123". Then it only
 *        has one delimiter; the string will be split to two substrings:
 *        E'\\\\\\\\\\%123' and '123'; the position array size is 1, where position[0] = 9;
 *        ; (*len) = 13.
 *
 * @param str       The string to be split.
 * @param position  An array to store the position of each un-escaped % in the string.
 * @param num_pos   The expected number of un-escaped %s in the string.
 * @param len       The length of the string. It doesn't include the terminal.
 *
 * @return The position array which records the positions of all un-escaped %s
 *         in the give string.
 *
 * @note If the number of %s in the string is not equal to the expected number,
 *       we will report error via elog.
 */
static
int*
dt_split_string
	(
	char *str,
	int  *position,
    int  num_pos,
	int  *len
	)
{
	int i 				  = 0;
	int j 				  = 0;
	
	/* the number of the escape chars which occur continuously */
	int num_cont_escapes  = 0;

	for(; str != NULL && *str != '\0'; ++str, ++j)
	{
		if ('%' == *str)
		{
			/*
			 * if the number of the escapes is even number
			 * then no need to escape. Otherwise escape the delimiter
			 */
			if (!(num_cont_escapes & 0x01))
			{
            	dt_check_error
	            	(
                        i < num_pos,
                        "the number of the elements in the array is less than "
                        "the format string expects."
                    );

				/*  convert the char '%' to '\0' */
				position[i++] 	= j;
				*str 			= '\0';
			}

			/* reset the number of the continuous escape chars */
			num_cont_escapes = 0;
		}
		else if ('\\' == *str)
		{
			/* increase the number of continuous escape chars */
			++num_cont_escapes;
		}
		else
		{
			/* reset the number of the continuous escape chars */
			num_cont_escapes = 0;
		}
	}

	*len      = j;
	
    dt_check_error
        (
            i == num_pos,
            "the number of the elements in the array is greater than "
            "the format string expects. "
        );

	return position;
}


/*
 * @brief Change all occurrences of '\%' in the give string to '%'. Our split
 *        method will ensure that the char immediately before a '%' must be a '\'.
 *        We traverse the string from left to right, if we meet a '%', then
 *        move the substring after the current '\%' to the right place until
 *        we meet next '\%' or the '\0'. Finally, set the terminal symbol for
 *        the replaced string.
 *
 * @param str   The null terminated string to be escaped.
 *              The char immediately before a '%' must be a '\'.
 *
 * @return The new string with \% changed to %.
 *
 */
static
char*
dt_escape_pct_sym
	(
	char *str
	)
{
	int num_escapes		  = 0;

	/* remember the start address of the escaped string */
	char *p_new_string 	 = str;

	while(str != NULL && *str != '\0')
	{
		if ('%' == *str)
		{
			dt_check_error_value
				(
					(str - 1) && ('\\' == *(str - 1)),
					"The char immediately before a %s must be a \\",
					"%"
				);

			/*
			 * The char immediately before % is \
			 * increase the number of escape chars
			 */
			++num_escapes;
			do
			{
				/*
				 * move the string which is between the current "\%"
				 * and next "\%"
				 */
				*(str - num_escapes) = *str;
				++str;
			} while (str != NULL && *str != '\0' && *str != '%');
		}
		else
		{
			++str;
		}
	}

	/* if there is no escapes, then set the end symbol for the string */
	if (num_escapes > 0)
		*(str - num_escapes) = '\0';

	return p_new_string;
}


/*
 * @brief We need to build a lot of query strings based on a set of arguments. For that
 *        purpose, this function will take a format string (the template) and an array
 *        of values, scan through the format string, and replace the %s in the format
 *        string with the corresponding values in the array. The result string is
 *        returned as a PG/GP text Datum. The escape char for '%' is '\'. And we will
 *        not change it's default behavior in PG/GP. For example, assume that
 *        fmt = E'\\\\\\\\ % \\% %', args[] = {"100", "20"}, then the returned text
 *        of this function is E'\\\\\\\\ 100 % 20'
 *
 * @param fmt       The format string. %s are used to indicate a position
 *                  where a value should be filled in.
 * @param args      An array of values that should be used for replacements.
 *                  args[i] replaces the i-th % in fmt. The array length should
 *                  equal to the number of %s in fmt.
 *
 * @return A string with all %s which were not escaped in first argument replaced
 *         with the corresponding values in the second argument.
 *
 */
Datum
dt_text_format
	(
	PG_FUNCTION_ARGS
	)
{
	dt_check_error
		(
			!(PG_ARGISNULL(0) || PG_ARGISNULL(1)),
			"the format string and its arguments must not be null"
		);

	char	   *fmt 		= text_to_cstring(PG_GETARG_TEXT_PP(0));
	ArrayType  *args_array 	= PG_GETARG_ARRAYTYPE_P(1);

    dt_check_error
		(
			!ARR_NULLBITMAP(args_array),
			"the argument array must not has null value"
		);

	int			nitems		= 0;
	int		   *dims		= NULL;
	int         ndims       = 0;
	Oid			element_type= 0;
	int			typlen		= 0;
	bool		typbyval	= false;
	char		typalign	= '\0';
	char	   *p			= NULL;
	int			i			= 0;

	ArrayMetaState *my_extra= NULL;
	StringInfoData buf;

	ndims  = ARR_NDIM(args_array);
	dims   = ARR_DIMS(args_array);
	nitems = ArrayGetNItems(ndims, dims);

	/* if there are no elements, return the format string directly */
	if (nitems == 0)
		PG_RETURN_TEXT_P(cstring_to_text(fmt));

	int *position 	= (int*)palloc0(nitems * sizeof(int));

	int last_pos    = 0;
	int len_fmt     = 0;

	/*
	 * split the format string, so that later we can replace the delimiters
	 * with the given arguments
	 */
	dt_split_string(fmt, position, nitems, &len_fmt);

	element_type = ARR_ELEMTYPE(args_array);
	initStringInfo(&buf);

	/*
	 * We arrange to look up info about element type, including its output
	 * conversion proc, only once per series of calls, assuming the element
	 * type doesn't change underneath us.
	 */
	my_extra = (ArrayMetaState *) fcinfo->flinfo->fn_extra;
	if (my_extra == NULL)
	{
		fcinfo->flinfo->fn_extra = MemoryContextAlloc
									(
										fcinfo->flinfo->fn_mcxt,
										sizeof(ArrayMetaState)
									);
		my_extra = (ArrayMetaState *) fcinfo->flinfo->fn_extra;
		my_extra->element_type = ~element_type;
	}

	if (my_extra->element_type != element_type)
	{
		/*
		 * Get info about element type, including its output conversion proc
		 */
		get_type_io_data
			(
				element_type,
				IOFunc_output,
				&my_extra->typlen,
				&my_extra->typbyval,
				&my_extra->typalign,
				&my_extra->typdelim,
				&my_extra->typioparam,
				&my_extra->typiofunc
			);
		fmgr_info_cxt
			(
				my_extra->typiofunc,
				&my_extra->proc,
				fcinfo->flinfo->fn_mcxt
			);
		my_extra->element_type = element_type;
	}
	typlen 		= my_extra->typlen;
	typbyval 	= my_extra->typbyval;
	typalign 	= my_extra->typalign;
	p 			= ARR_DATA_PTR(args_array);

	for (i = 0; i < nitems; i++)
	{
		Datum		itemvalue;
		char	   *value;

		itemvalue = fetch_att(p, typbyval, typlen);
		value 	  = OutputFunctionCall(&my_extra->proc, itemvalue);

		/* there is no string before the delimiter */
		if (last_pos == position[i])
		{
			appendStringInfo(&buf, "%s", value);
			++last_pos;
		}
		else
		{
			/*
			 * has a string before the delimiter
			 * we replace "\%" in the string to "%", since "%" is escaped
			 * then combine the string and argument string together
			 */
			appendStringInfo
				(
					&buf,
					"%s%s",
					dt_escape_pct_sym(fmt + last_pos),
					value
				);

			last_pos = position[i] + 1;
		}

		p = att_addlength_pointer(p, typlen, p);
		p = (char *) att_align_nominal(p, typalign);
	}

	/* the last char in the format string is not delimiter */
	if (last_pos < len_fmt)
		appendStringInfo(&buf, "%s", fmt + last_pos);

	PG_RETURN_TEXT_P(cstring_to_text_with_len(buf.data, buf.len));
}
PG_FUNCTION_INFO_V1(dt_text_format);


/*
 * @brief This function checks whether the specified table exists or not.
 *
 * @param input     The table name to be tested.	    
 *
 * @return A boolean value indicating whether the table exists or not.
 */
Datum table_exists(PG_FUNCTION_ARGS)
{
    text*           input;
    List*           names;
    Oid             relid;

    if (PG_ARGISNULL(0))
        PG_RETURN_BOOL(false);
    
    input = PG_GETARG_TEXT_PP(0);

    names = textToQualifiedNameList(input);
    relid = RangeVarGetRelid(makeRangeVarFromNameList(names), true);
    PG_RETURN_BOOL(OidIsValid(relid));
}
PG_FUNCTION_INFO_V1(table_exists);

