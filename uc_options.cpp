/*
 * UniversalContainer library.
 * Copyright Jason Denton, 2008,2010.
 * Made available under the new BSD license, as described in LICENSE
 *
 * Send comments and bug reports to jason.denton@gmail.com
 * http://www.greatpanic.com/code.html
 */

//note: the only public function here is parse_command_line
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "ucontainer.h"
#include "ucio.h"
#include "file_adapter.h"
#include "buffer_adapter.h"


namespace JAD {

	void show_help(const CmdLineOption* options, const char* text)
	{
		const CmdLineOption* opt = options;
		printf("%s\n\n", text);
		while (opt->lng) {
			printf("\t-%c, --%s\t\t%s\n\t\t\t\t\t(default: %s)\n",
				opt->shrt, opt->lng, opt->desc, opt->def);
			opt++;
		}
		exit(0);
	}

	UniversalContainer set_option_defaults(const CmdLineOption* options)
	{
		UniversalContainer defaults;
		const CmdLineOption* option = options;

		while (option->lng) {
			defaults[option->lng].string_interpret(option->def);
			option++;
		}
		return defaults;
	}

	UniversalContainer finalize_options(UniversalContainer params, const CmdLineOption* options)
	{
		const CmdLineOption* option;
		bool valid = true;
		FILE* afile = NULL;
		Buffer* buf;

		for (option = options; option->lng; option++) {
			if (option->ptype == uc_FileInName || 
				option->ptype == uc_Buffer || 
				option->ptype == uc_JSON) {
				if (strcmp(params[option->lng].c_str(),"stdin") != 0)
					afile = fopen(params[option->lng].c_str(), "r");
				else
					afile = stdin;

				if (!afile) {
					fprintf(stderr, "Error: Unable to open file %s for input.\n",
						params[option->lng].c_str());
					valid = false;
					continue;
				}
				if (option->ptype == uc_FileInName) { 
					params[option->lng].clear();
					params[option->lng] = FILEAdapter::assign(afile);
					continue;
				}
				buf = read_to_buffer(afile);
				if (strcmp(params[option->lng].c_str(),"stdin") != 0) fclose(afile);
				if (!buf) {
					fprintf(stderr, "Error: Failed to read file %s.\n", 
						params[option->lng].c_str());
					valid = false;
					continue;
				}
				if (option->ptype == uc_Buffer) { 
					params[option->lng].clear();
					params[option->lng] = BufferAdapter::assign(buf); 
					continue;
				}
				params[option->lng].clear();
				try {
					params[option->lng] = uc_decode_json(buf);
				} catch (UniversalContainer ex) {
					if (ex["code"] == uce_Deserialization_Error) {
						fprintf(stderr, "Error: File %s does not contain valid JSON.\n", 
							params[option->lng].c_str());
						valid = false;
						continue;
					}
					throw ex; //not deserialization, shouldn't get here.
				}
			} else if(option->ptype == uc_FileOutName) {
				if (strcmp(params[option->lng].c_str(),"stdout") != 0) 
					afile = fopen(params[option->lng].c_str(), "w");
				else
					afile = stdout;

				if (!afile) {
					fprintf(stderr, "Error: Unable to open file %s for output.\n", 
						params[option->lng].c_str());
					valid = false;
					continue;
				}
				params[option->lng].clear();
				params[option->lng] = FILEAdapter::assign(afile);		
			} else if ((option->ptype == uc_Integer || 
				option->ptype == uc_Character || option->ptype == uc_String ||
				option->ptype == uc_WString || option->ptype == uc_Real) &&
				option->ptype != params[option->lng].get_type()) {
					fprintf(stderr, "Error: Wrong argument type for %s.\n", option->lng);
					valid = false;
					continue;
			}	
		} //end for
		if (!valid) exit(1);
		return params;
	}

	const CmdLineOption* find_short_option(char param, const CmdLineOption* options)
	{
		const CmdLineOption* opt = options;
		while (opt->lng && opt->shrt != param) opt++;
		if (!opt->lng) return NULL;
		return opt;
	}

	const CmdLineOption* find_option(char* param, const CmdLineOption* options)
	{
		const CmdLineOption* opt = options;
		if (param[0] != '-') return NULL;
		if (param[1] != '-') return find_short_option(param[1], options);
		param += 2;
		while (opt->lng && strcmp(opt->lng, param)) opt++;
		if (!opt->lng) return NULL;
		return opt;
	}

	UniversalContainer parse_command_line(int argc, char** argv, 
		const CmdLineOption* options, const char* text)
	{
		UniversalContainer params = set_option_defaults(options);
		params["_remaining"].init_array();
		params["_executable"] = argv[0];

		const CmdLineOption* opt = NULL;
		try {
			for (int i = 1; i < argc; i++) {
				if (!strcmp("--help", argv[i]) || !strcmp("-h", argv[1]))
					show_help(options, text); //show_help does exit(0)
				opt = find_option(argv[i], options);
				if (!opt) {
					params["_remaining"][-1] = argv[i];
					continue;
				}
				if (opt->ptype == uc_Boolean) 
					params[opt->lng] = !bool(params[opt->lng]);
				else {
					UniversalContainer tmp;
					i++;
					tmp.string_interpret(argv[i]);
					params[opt->lng] = tmp;
				}
			}
		} catch (UniversalContainer ex) {
			fprintf(stderr, "Error: Parameter %s must be of type %s.\n", 
				opt->lng, UnviersalContainerTypeName[int(opt->ptype)]);
			exit(1);
		}
		opt = options;
		while (opt->lng) {
			if (params[opt->lng].get_type() == uc_String &&
				strcmp(params[opt->lng].c_str(), "__REQUIRED__") == 0)
				show_help(options, text);
			opt++;
		}
					
		return finalize_options(params, options);
	}

} //end namespace
