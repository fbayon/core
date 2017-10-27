/*
 *	Reflect Library by Parra Studios
 *	Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	A library for provide reflection and metadata representation.
 *
 */

#include <reflect/reflect_signature.h>

#include <reflect/reflect_value_type.h>

#include <log/log.h>

#include <stdlib.h>
#include <string.h>

typedef struct signature_node_type
{
	char *			name;
	type			t;
} * signature_node;

struct signature_type
{
	type			ret;
	size_t			count;
};

static signature_node signature_head(signature s);

static signature_node signature_at(signature s, size_t index);

static value signature_metadata_return(signature s);

static value signature_metadata_args_map(signature s);

static value signature_metadata_args(signature s);

signature_node signature_head(signature s)
{
	if (s != NULL)
	{
		return (signature_node)((signature)(s + 1));
	}

	return NULL;
}

signature_node signature_at(signature s, size_t index)
{
	signature_node node_list = signature_head(s);

	if (node_list != NULL && index < s->count)
	{
		return &node_list[index];
	}

	return NULL;
}

signature signature_create(size_t count)
{
	signature s = malloc(sizeof(struct signature_type) + sizeof(struct signature_node_type) * count);

	if (s != NULL)
	{
		size_t index;

		s->ret = NULL;

		s->count = count;

		for (index = 0; index < count; ++index)
		{
			signature_node node = signature_at(s, index);

			if (node != NULL)
			{
				node->name = NULL;
				node->t = NULL;
			}
		}

		return s;
	}

	return NULL;
}

size_t signature_count(signature s)
{
	if (s != NULL)
	{
		return s->count;
	}

	return 0;
}

const char * signature_get_name(signature s, size_t index)
{
	if (s != NULL && index < s->count)
	{
		signature_node node = signature_at(s, index);

		return node->name;
	}

	return NULL;
}

type signature_get_type(signature s, size_t index)
{
	if (s != NULL && index < s->count)
	{
		signature_node node = signature_at(s, index);

		return node->t;
	}

	return NULL;
}

type signature_get_return(signature s)
{
	if (s != NULL)
	{
		return s->ret;
	}

	return NULL;
}

void signature_set(signature s, size_t index, const char * name, type t)
{
	if (s != NULL && index < s->count && name != NULL)
	{
		signature_node node = signature_at(s, index);

		size_t name_size = strlen(name) + 1;

		char * name_node = malloc(sizeof(char) * name_size);

		if (name_node == NULL)
		{
			log_write("metacall", LOG_LEVEL_ERROR, "Invalid name allocation");

			return;
		}

		if (node->name != NULL)
		{
			free(node->name);
		}

		node->name = name_node;

		memcpy(node->name, name, name_size);

		node->t = t;
	}
}

void signature_set_return(signature s, type t)
{
	if (s != NULL)
	{
		s->ret = t;
	}
}

value signature_metadata_return(signature s)
{
	static const char ret_str[] = "ret";

	const char * ret_name = s->ret != NULL ? type_name(s->ret) : "";

	value ret = value_create_array(NULL, 2);

	value * ret_array;

	if (ret == NULL)
	{
		return NULL;
	}

	ret_array = value_to_array(ret);

	ret_array[0] = value_create_string(ret_str, sizeof(ret_str) - 1);

	if (ret_array[0] == NULL)
	{
		/*
		value_type_destroy(ret);
		*/

		return NULL;
	}

	ret_array[1] = value_create_string(ret_name, strlen(ret_name));

	if (ret_array[1] == NULL)
	{
		/*
		value_type_destroy(ret);
		*/

		return NULL;
	}

	return ret;
}

value signature_metadata_args_map(signature s)
{
	value args = value_create_map(NULL, s->count);

	if (args == NULL)
	{
		return NULL;
	}

	if (s->count > 0)
	{
		value * args_map = value_to_map(args);

		size_t index;

		for (index = 0; index < s->count; ++index)
		{
			signature_node node = signature_at(s, index);

			const char * type_str = type_name(node->t);

			value * args_map_array;

			args_map[index] = value_create_array(NULL, 2);

			if (args_map[index] == NULL)
			{
				/*
				value_type_destroy(args);
				*/

				return NULL;
			}

			args_map_array = value_to_array(args_map[index]);

			args_map_array[0] = value_create_string(node->name, strlen(node->name));

			if (args_map_array[0] == NULL)
			{
				/*
				value_type_destroy(args);
				*/

				return NULL;
			}

			args_map_array[1] = value_create_string(type_str, strlen(type_str));

			if (args_map_array[1] == NULL)
			{
				/*
				value_type_destroy(args);
				*/

				return NULL;
			}
		}
	}

	return args;
}

value signature_metadata_args(signature s)
{
	static const char args_str[] = "args";

	value args_map = signature_metadata_args_map(s);

	if (args_map != NULL)
	{
		value args = value_create_array(NULL, 2);

		value * args_array;

		if (args == NULL)
		{
			/*
			value_type_destroy(args);
			*/

			return NULL;
		}

		args_array = value_to_array(args);

		args_array[0] = value_create_string(args_str, sizeof(args_str) - 1);

		if (args_array[0] == NULL)
		{
			/*
			value_type_destroy(args);
			*/

			return NULL;
		}

		args_array[1] = args_map;

		return args;
	}

	return NULL;
}


value signature_metadata(signature s)
{
	value ret, args, sig;

	value * sig_map;

	/* Create return array */
	ret = signature_metadata_return(s);

	if (ret == NULL)
	{
		return NULL;
	}

	/* Create arguments map */
	args = signature_metadata_args(s);

	if (args == NULL)
	{
		/*
		value_destroy_type(ret);
		*/

		return NULL;
	}

	/* Create signature map (return type + args) */
	sig = value_create_map(NULL, 2);

	if (sig == NULL)
	{
		/*
		value_type_destroy(ret);

		value_type_destroy(args);
		*/

		return NULL;
	}

	sig_map = value_to_map(sig);

	sig_map[0] = ret;

	sig_map[1] = args;

	return sig;
}

void signature_destroy(signature s)
{
	if (s != NULL)
	{
		size_t index;

		for (index = 0; index < s->count; ++index)
		{
			signature_node node = signature_at(s, index);

			if (node != NULL && node->name != NULL)
			{
				free(node->name);
			}
		}

		free(s);
	}
}
