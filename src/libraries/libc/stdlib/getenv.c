#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define ENV_HOLE (char *)0xcc00

char **environ;

static bool is_valid_envname(const char *envname)
{
	if (!envname)
		return false;

	for (int i = 0, length = strlen(envname); i < length; ++i)
	{
		if (envname[i] == '=')
			return false;
	}
	return true;
}

static bool is_valid_env(const char *env)
{
	for (int i = 0, length = strlen(env); i < length; ++i)
	{
		if (env[i] == '=')
			return true;
	}
	return false;
}

char *construct_env(const char *envname, const char *envval)
{
	int name_length = strlen(envname);
	int val_length = strlen(envval);
	int length = name_length + val_length + 1;

	char *env = calloc(length + 1, sizeof(char));

	int i = 0;
	for (; i < name_length; ++i)
	{
		env[i] = envname[i];
	}

	env[i++] = '=';

	for (int j = 0; j < val_length; ++i, ++j)
	{
		env[i] = envval[j];
	}

	return env;
}

int add_new_env(const char *env, int length)
{
	for (int i = 0; i < length; ++i)
	{
		if (environ[i] == ENV_HOLE)
		{
			environ[i] = env;
			return 0;
		}
	}

	if (length >= ARG_MAX)
		return errno = ENOMEM, -1;

	char **old_environ = environ;
	environ = calloc(length + 2, sizeof(char *));
	for (int i = 0; i < length; ++i)
	{
		environ[i] = strdup(old_environ[i]);
		free(old_environ[i]);
	}
	environ[length] = env;
	return 0;
}

char *getenv(const char *name)
{
	if (!is_valid_envname(name))
		return errno = EINVAL, NULL;

	for (int i = 0, env_length = count_array_of_pointers(environ); i < env_length; ++i)
	{
		char *env = environ[i];
		if (env == ENV_HOLE)
			continue;

		for (int j = 0; env[j]; ++j)
		{
			if (env[j] == '=')
				return name[j] == 0 ? env + j + 1 : NULL;
			else if (env[j] != name[i])
				break;
		}
	}
	return NULL;
}

int setenv(const char *envname, const char *envval, int overwrite)
{
	if (!is_valid_envname(envname))
		return errno = EINVAL, -1;

	int env_length = count_array_of_pointers(environ);
	for (int i = 0; i < env_length; ++i)
	{
		char *env = environ[i];
		if (env == ENV_HOLE)
			continue;

		for (int j = 0; env[j]; ++j)
		{
			if (env[j] == '=')
			{
				if (envname[j] == 0)
				{
					if (overwrite)
					{
						free(environ[i]);
						environ[i] = construct_env(envname, envval);
					}
					return 0;
				}
				break;
			}
			else if (env[j] != envname[i])
				break;
		}
	}

	return add_new_env(construct_env(envname, envval), env_length);
}

int putenv(char *string)
{
	if (!is_valid_env(string))
		return -1;

	int env_length = count_array_of_pointers(environ);
	for (int i = 0; i < env_length; ++i)
	{
		char *env = environ[i];
		if (env == ENV_HOLE)
			continue;

		for (int j = 0; env[j]; ++j)
		{
			if (env[j] == '=')
			{
				if (string[j] == '=')
				{
					free(environ[i]);
					environ[i] = string;
					return 0;
				}
				else
					break;
			}
			else if (env[j] != string[j])
				break;
		}
	}
	return add_new_env(string, env_length);
}

int unsetenv(const char *name)
{
	if (!is_valid_envname(name))
		return errno = EINVAL, -1;

	for (int i = 0, length = count_array_of_pointers(environ); i < length; ++i)
	{
		char *env = environ[i];
		for (int j = 0; env[j]; ++j)
		{
			if (env[j] == '=')
			{
				if (name[j] == 0)
				{
					environ[j] = ENV_HOLE;
					return 0;
				}
				break;
			}
			else if (env[j] != name[j])
				break;
		}
	}
	return 0;
}
