#ifdef __clang__
    #pragma clang diagnostic error "-Wshadow-all"
    #pragma clang diagnostic error "-Wreturn-type"
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
#define OFD_OS_WINDOWS
#endif

#ifdef OFD_OS_WINDOWS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#define ofd_sprintf stbsp_sprintf
#define OFD_IMPLEMENTATION
#include "ofd.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef uint16_t u16;
typedef uint32_t u32;


#ifdef OFD_OS_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#pragma comment(lib, "kernel32")

#else
/////////////////////////
// NOTE: we assume POSIX.
/////////////////////////

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#endif // OFD_OS_WINDOWS


int ofd_utf16_strlen(u16* s)
{
    int count = 0;
    while(1)
    {
        if(*s == 0) return count;
        count++;
        s++;
    }
    
    return count;
}

void ofd_utf8_to_utf16(char* utf8_string, u16* result)
{
    int utf8_count = strlen(utf8_string);
    
    int result_size = utf8_count * sizeof(u16) * 2 + 2;
    memset(result, 0, result_size);
    
    u16* output = result;
    
    char* input = utf8_string;
    char* limit = utf8_string + utf8_count;
    
    while(input < limit)
    {
        u32 codepoint = 0;
        
        // Retrieve codepoint. START
        if((input[0] & 0b10000000) == 0)
        {
            codepoint = input[0];
            input++;
        }
        else if((input[0] & 0b11100000) == 0b11000000)
        {
            codepoint = (ofd_cast(input[0] & 0b11111, u32) << 6) | (input[1] & 0b111111);
            input += 2;
        }
        else if((input[0] & 0b11110000) == 0b11100000)
        {
            codepoint = (ofd_cast(input[0] & 0b1111, u32) << 12) | (ofd_cast(input[1] & 0b111111, u32) << 6) | (input[2] & 0b111111);
            input += 3;
        }
        else
        {
            codepoint = (ofd_cast(input[0] & 0b11111, u32) << 18) | (ofd_cast(input[1] & 0b111111, u32) << 12) | (ofd_cast(input[2] & 0b111111, u32) << 6) | (input[3] & 0b111111);
            input += 4;
        }
        // Retrieve codepoint. END
        
        // Convert codepoint to UTF-16. START
        if(codepoint < 0xd800 || (codepoint >= 0xe000 && codepoint <= 0xffFF))
        {
            *output = codepoint;
            output++;
        }
        else
        {
            codepoint -= 0x10000;
            
            output[0] = (codepoint >> 10) + 0xd800;
            output[1] = (codepoint & 0b1111111111) + 0xdc00;
            output += 2;
        }
        // Convert codepoint to UTF-16. END
    }
    
    *output = 0;
}

int ofd_utf16_to_utf8(u16* input, char* output)
{
    /////////////////////////////////////////////////////////////
    // NOTE: this returns the size of the resulting UTF-8 string.
    /////////////////////////////////////////////////////////////
    
    int input_size = ofd_utf16_strlen(input);
    int output_size = input_size * 4 + 1;
    
    u16* limit = input + input_size;
    
    memset(output, 0, output_size);
    
    int result_count = 0;
    
    int num_utf16_characters_decoded_so_far = 0;
    while(input < limit)
    {
        u32 codepoint = 0;
        u16 value = *input;
        
        // Retrieve codepoint. START
        if(value < 0xd800 || (value >= 0xe000 && value <= 0xffFF))
        {
            codepoint = value;
            input++;
        }
        else
        {
            u16 leading_surrogate  = input[0];
            u16 trailing_surrogate = input[1];
            
            if((leading_surrogate & 0b1111110000000000) == 0b1101100000000000 && (trailing_surrogate & 0b1111110000000000) == 0b1101110000000000)
            {
                codepoint = (ofd_cast(leading_surrogate - 0xd800, u32) << 10) | (trailing_surrogate - 0xdc00);
                input += 2;
            }
            else input++;
        }
        // Retrieve codepoint. END
        
        // Convert codepoint to UTF-8. START
        if(codepoint < 0x80)
        {
            output[0] = codepoint;
            output++;
            result_count++;
        }
        else if(codepoint < 0x800)
        {
            output[0] = 0b11000000 | (codepoint >> 6);
            output[1] = 0b10000000 | (codepoint & 0b111111);
            
            output       += 2;
            result_count += 2;
        }
        else if(codepoint < 0x10000)
        {
            output[0] = 0b11100000 | (codepoint >> 12);
            output[1] = 0b10000000 | ((codepoint >> 6) & 0b111111);
            output[2] = 0b10000000 | (codepoint & 0b111111);
            
            output       += 3;
            result_count += 3;
        }
        else
        {
            output[0] = 0b11110000 | (codepoint >> 18);
            output[1] = 0b10000000 | ((codepoint >> 12) & 0b111111);
            output[2] = 0b10000000 | ((codepoint >> 6) & 0b111111);
            output[3] = 0b10000000 | (codepoint & 0b111111);
            
            output       += 4;
            result_count += 4;
        }
        // Convert codepoint to UTF-8. END
    }
    
    *output = 0;
    return result_count;
}


typedef struct
{
    int id;
    int filepath_count;
    char filepath_memory[2048]; // This must always be usable as a C string.
} Ofd_File;

void ofd_put_last_file_in_the_right_place(Ofd_Array* files)
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE: for now we sort files in ASCII order. No particular attention is given to numbers which means the following will happen:
    // 'hello10' < 'hello4'
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    if(files->count == 1) return;
    
    Ofd_File* file = ofd_cast(files->data, Ofd_File*) + files->count - 1;
    
    int higher_bound  = files->count - 1;
    int lower_bound   = 0;
    int current_index = 0;
    
    while(1)
    {
        ////////////////////////////////////////////////////////////////////////////////////////
        // NOTE: we just binary search the array to find the best place to put the last file in.
        ////////////////////////////////////////////////////////////////////////////////////////
        
        current_index = lower_bound + (higher_bound - lower_bound) / 2;
        Ofd_File* current_file = ofd_cast(files->data, Ofd_File*) + current_index;
        
        
        ofd_b8 we_should_go_below = file->id < current_file->id;
        
        if(we_should_go_below) higher_bound = current_index;
        else                   lower_bound  = current_index + 1;
        
        if(lower_bound >= higher_bound) break;
    }
    
    current_index = higher_bound;
    if(current_index < files->count - 1)
    {
        Ofd_File tmp = *file;
        Ofd_File* where_to_move = ofd_cast(files->data, Ofd_File*) + current_index;
        int num_files_above = files->count - current_index - 1;
        
        memmove(where_to_move + 1, where_to_move, num_files_above * sizeof(Ofd_File));
        memcpy(where_to_move, &tmp, sizeof(Ofd_File));
    }
}

void ofd_add_markdown_file(Ofd_Array* files, char* directory_path, char* c_filename)
{
    Ofd_String filename = Ofd_String_(c_filename);
    int file_id = 0;
    
    { // Get the file ID.
        char* c     = filename.data;
        char* limit = filename.data + filename.count;
        
        Ofd_String id = {c};
        while(c < limit)
        {
            if(*c < '0' || *c > '9') break;
            c++;
        }
        
        id.count = c - id.data;
        
        char id_c_string[8];
        ofd_to_c_string(id, id_c_string);
        
        file_id = atoi(id_c_string);
    }
    
    
    Ofd_File* file = ofd_array_add_fast(files);
    
    stbsp_sprintf(file->filepath_memory, "%s/%s", directory_path, c_filename);
    file->filepath_count = strlen(file->filepath_memory);
    file->id = file_id;
    
    
    // Put the file in the right place. START
    if(files->count == 1) return;
    
    int higher_bound  = files->count - 1;
    int lower_bound   = 0;
    int current_index = 0;
    
    while(1)
    {
        ////////////////////////////////////////////////////////////////////////////////////////
        // NOTE: we just binary search the array to find the best place to put the last file in.
        ////////////////////////////////////////////////////////////////////////////////////////
        
        current_index = lower_bound + (higher_bound - lower_bound) / 2;
        Ofd_File* current_file = ofd_cast(files->data, Ofd_File*) + current_index;
        
        
        ofd_b8 we_should_go_below = file->id < current_file->id;
        
        if(we_should_go_below) higher_bound = current_index;
        else                   lower_bound  = current_index + 1;
        
        if(lower_bound >= higher_bound) break;
    }
    
    current_index = higher_bound;
    if(current_index < files->count - 1)
    {
        Ofd_File tmp = *file;
        Ofd_File* where_to_move = ofd_cast(files->data, Ofd_File*) + current_index;
        int num_files_above = files->count - current_index - 1;
        
        memmove(where_to_move + 1, where_to_move, num_files_above * sizeof(Ofd_File));
        memcpy(where_to_move, &tmp, sizeof(Ofd_File));
    }
    // Put the file in the right place. END
}

Ofd_Array ofd_os_list_markdown_files(char* directory_path)
{
    Ofd_Array files;
    ofd_array_init(&files, sizeof(Ofd_File));
    
    char path_to_search[2048];
    
    #ifdef OFD_OS_WINDOWS
    // @ Report an error if this is not a directory.
    
    stbsp_sprintf(path_to_search, "%s/*.md", directory_path);
    
    WCHAR utf16_path_to_search[2048];
    ofd_utf8_to_utf16(path_to_search, utf16_path_to_search);
    
    WIN32_FIND_DATAW info = {0};
    HANDLE search_handle = FindFirstFileW(utf16_path_to_search, &info);
    if(search_handle == INVALID_HANDLE_VALUE) return files; // There does not seem to be anything in this directory.
    
    while(1)
    {
        char utf8_name[2048];
        ofd_utf16_to_utf8(info.cFileName, utf8_name);
        
        /*Ofd_File* file = ofd_array_add_fast(&files);
        
        stbsp_sprintf(file->filepath_memory, "%s/%s", directory_path, utf8_name);
        file->filepath_count = strlen(file->filepath_memory);
        
        ofd_put_last_file_in_the_right_place(&files);*/
        
        ofd_add_markdown_file(&files, directory_path, utf8_name);
        
        BOOL status = FindNextFileW(search_handle, &info);
        if(!status) break;
    }
    
    FindClose(search_handle);
    #else
    DIR* directory = opendir(directory_path);
    if(!directory) return files; // We failed to open the directory.
    
    while(1)
    {
        struct dirent* info = readdir(directory);
        if(!info) break; // We listed everything in the directory.
        
        
        Ofd_String filename = Ofd_String_(info->d_name);
        
        char* limit = filename.data + filename.count;
        char* c     = limit - 1;
        while(c >= filename.data)
        {
            if(*c == '.') break;
            c--;
        }
        
        c++;
        Ofd_String extension = {c, limit - c};
        if(!ofd_string_matches(extension, "md")) continue; // This is not a markdown file.
        
        
        Ofd_File* file = ofd_array_add_fast(&files);
        
        stbsp_sprintf(file->filepath_memory, "%s/%s", directory_path, info->d_name);
        file->filepath_count = strlen(file->filepath_memory);
        
        ofd_put_last_file_in_the_right_place(&files);
    }
    #endif
    
    
    return files;
}

ofd_b8 ofd_os_file_exists(char* filepath)
{
    #ifdef OFD_OS_WINDOWS
    WCHAR utf16_name[2048];
    ofd_utf8_to_utf16(filepath, utf16_name);
    
    WIN32_FIND_DATAW info = {0};
    HANDLE search_handle = FindFirstFileW(utf16_name, &info);
    if(search_handle == INVALID_HANDLE_VALUE) return ofd_false;
    
    FindClose(search_handle);
    return ofd_true;
    #else
    return (access(filepath, F_OK) == 0);
    #endif
    
    return ofd_false;
}


int main(int num_arguments, char** arguments)
{
    const char* HELP_MESSAGE =
        "Usage: ofd <options> <input files>\n"
        "\n"
        "Options:\n"
        "-o, -output <file>:   specifies the output filepath. The default is 'result.html'.\n"
        "-t, -theme  <file>:   specifies a theme file. If no theme is provided a default theme is applied.\n"
        "-l, -logo   <path>:   specifies a path to a logo.\n"
        "-i, -icon   <path>:   specifies a path to an icon.\n"
        "-title      <title>:  specifies a title.\n"
        "-c, -config <file>:   specifies a project configuration file.\n"
        "\n"
        "-d, -dir <directory>: specifies a directory to build documentation from.\n"
        "                      ofd takes all .md files in the directory as input.\n"
        "                      You can also put an implicit project configuration file named .ofd in the directory.\n"
        "                      Note that sub-directories are not searched.\n"
        "\n"
        "-h, -help:            shows information on how to use this program.\n"
    ;
    
    if(num_arguments > 1)
    {
        char* html_filepath     = NULL;
        char* theme_filepath    = NULL;
        char* logo_path         = NULL;
        char* icon_path         = NULL;
        char* config_filepath   = NULL;
        char* directory_path    = NULL;
        char* title             = NULL;
        
        Ofd_Array md_files;
        ofd_array_init(&md_files, sizeof(char*));
        
        int arg_index = 1;
        while(arg_index < num_arguments)
        {
            char* argument = arguments[arg_index];
            
            if(strcmp(argument, "-c") == 0 || strcmp(argument, "-config") == 0)
            {
                // Retrieve config filepath. START
                if(config_filepath)
                {
                    printf("It looks like you already specified the configuration file to be '%s'.", config_filepath);
                    return -1;
                }
                
                if(arg_index == num_arguments - 1)
                {
                    printf("It looks like you forgot to specify the configuration file filepath after the '%s' switch.\n", argument);
                    return -1;
                }
                
                config_filepath = arguments[arg_index + 1];
                // Retrieve config filepath. END
                
                arg_index += 2;
                continue;
            }
            else if(strcmp(argument, "-d") == 0 || strcmp(argument, "-dir") == 0)
            {
                // Retrieve a directory path. START
                if(directory_path)
                {
                    printf("It looks like you already specified the directory to make documentation from to be '%s'.", directory_path);
                    return -1;
                }
                
                if(arg_index == num_arguments - 1)
                {
                    printf("It looks like you forgot to specify a directory path after the '%s' switch.\n", argument);
                    return -1;
                }
                
                directory_path = arguments[arg_index + 1];
                // Retrieve a directory path filepath. END
                
                arg_index += 2;
                continue;
            }
            else if(strcmp(argument, "-i") == 0 || strcmp(argument, "-icon") == 0)
            {
                // Retrieve an icon path. START
                if(icon_path)
                {
                    printf("It looks like you already specified the icon path to be '%s'.", icon_path);
                    return -1;
                }
                
                if(arg_index == num_arguments - 1)
                {
                    printf("It looks like you forgot to specify an icon path after the '%s' switch.\n", argument);
                    return -1;
                }
                
                icon_path = arguments[arg_index + 1];
                // Retrieve an icon path. END
                
                arg_index += 2;
                continue;
            }
            else if(strcmp(argument, "-title") == 0)
            {
                // Retrieve a title. START
                if(title)
                {
                    printf("It looks like you already specified the title to be '%s'.", title);
                    return -1;
                }
                
                if(arg_index == num_arguments - 1)
                {
                    printf("It looks like you forgot to specify a title after the '%s' switch.\n", argument);
                    return -1;
                }
                
                title = arguments[arg_index + 1];
                // Retrieve a title. END
                
                arg_index += 2;
                continue;
            }
            else if(strcmp(argument, "-o") == 0 || strcmp(argument, "-output") == 0)
            {
                // Retrieve output filepath. START
                if(html_filepath)
                {
                    printf("It looks like you already specified the result filepath to be '%s'.", html_filepath);
                    return -1;
                }
                
                if(arg_index == num_arguments - 1)
                {
                    printf("It looks like you forgot to specify the result filepath after the '%s' switch.\n", argument);
                    return -1;
                }
                
                html_filepath = arguments[arg_index + 1];
                // Retrieve output filepath. END
                
                arg_index += 2;
                continue;
            }
            else if(strcmp(argument, "-t") == 0 || strcmp(argument, "-theme") == 0)
            {
                // Retrieve theme file. START
                if(theme_filepath)
                {
                    printf("It looks like you already specified the theme file to be '%s'.", theme_filepath);
                    return -1;
                }
                
                if(arg_index == num_arguments - 1)
                {
                    printf("It looks like you forgot to specify a theme file after the '%s' switch.\n", argument);
                    return -1;
                }
                
                theme_filepath = arguments[arg_index + 1];
                // Retrieve theme file. END
                
                arg_index += 2;
                continue;
            }
            else if(strcmp(argument, "-l") == 0 || strcmp(argument, "-logo") == 0)
            {
                // Retrieve logo path. START
                if(logo_path)
                {
                    printf("It looks like you already specified the logo path to be '%s'.", logo_path);
                    return -1;
                }
                
                if(arg_index == num_arguments - 1)
                {
                    printf("It looks like you forgot to specify a logo path after the '%s' switch.\n", argument);
                    return -1;
                }
                
                logo_path = arguments[arg_index + 1];
                // Retrieve logo path. END
                
                arg_index += 2;
                continue;
            }
            else if(strcmp(argument, "-h") == 0 || strcmp(argument, "-help") == 0)
            { // Display a help message.
                printf("%s", HELP_MESSAGE);
                return -1;
            }
            else
            {
                char** md_file = ofd_array_add_fast(&md_files);
                *md_file = argument;
                
                arg_index++;
            }
        }
        
        ofd_b8 update_config_filepaths_because_of_directory = ofd_false;
        char implicit_config_filepath[1024];
        
        if(directory_path)
        {
            // Retrieve markdown files from a directory. START
            if(md_files.count)
            {
                printf("You specified a directory to make documentation from but you also specified markdown files.\nYou can specify either one but not both at the same time.");
                return -1;
            }
            
            Ofd_Array files = ofd_os_list_markdown_files(directory_path);
            for(int i = 0; i < files.count; i++)
            {
                Ofd_File* file = ofd_cast(files.data, Ofd_File*) + i;
                char** filepath = ofd_array_add_fast(&md_files);
                *filepath = file->filepath_memory;
            }
            
            
            // Check wether we should also load a configuration file from this directory. START
            stbsp_sprintf(implicit_config_filepath, "%s/.ofd", directory_path);
            
            if(ofd_os_file_exists(implicit_config_filepath))
            {
                if(config_filepath)
                {
                    printf("You specified a configuration file but there is an implicit .ofd configuration file in the directory you want to make documentation from.\nEither delete this .ofd file or do not explicitly set a configuration file.");
                    return -1;
                }
                
                config_filepath = implicit_config_filepath;
                update_config_filepaths_because_of_directory = ofd_true;
            }
            // Check wether we should also load a configuration file from this directory. END
            // Retrieve markdown files from a directory. END
        }
        else directory_path = "";
        
        if(config_filepath)
        {
            // Parse the configuration file. START
            Ofd_String config_data;
            ofd_b8 status = ofd_read_whole_file(config_filepath, &config_data);
            if(!status) return 0;
            
            Ofd_Config config = {0};
            
            char error_message[1024];
            status = ofd_parse_configuration_data(config_data.data, config_data.count, &config, error_message);
            if(!status)
            {
                printf("Failed to parse configuration file '%s' with the following error:\n%s", config_filepath, error_message);
                return -1;
            }
            
            
            int directory_path_count = strlen(directory_path);
            
            int total_string_data_size = config.output_filepath.count + config.theme_filepath.count + config.logo_path.count + config.icon_path.count + config.title.count;
            if(total_string_data_size && directory_path_count) total_string_data_size += directory_path_count * 2;
            
            char* string_data = malloc(total_string_data_size + 5 * (total_string_data_size != 0));
            int offset = 0;
            
            char tmp_buffer[2048];
            
            if(config.output_filepath.count && !html_filepath)
            {
                if(directory_path_count) 
                {
                    char c_string[2048];
                    ofd_to_c_string(config.output_filepath, c_string);
                    stbsp_sprintf(tmp_buffer, "%s/%s", directory_path, c_string);
                }
                else ofd_to_c_string(config.output_filepath, tmp_buffer);
                
                int size = strlen(tmp_buffer) + 1;
                html_filepath = malloc(size);
                memcpy(html_filepath, tmp_buffer, size);
            }
            if(config.theme_filepath.count && !theme_filepath)
            {
                if(directory_path_count)
                {
                    char c_string[2048];
                    ofd_to_c_string(config.theme_filepath, c_string);
                    stbsp_sprintf(tmp_buffer, "%s/%s", directory_path, c_string);
                }
                else ofd_to_c_string(config.theme_filepath, tmp_buffer);
                
                int size = strlen(tmp_buffer) + 1;
                theme_filepath = malloc(size);
                memcpy(theme_filepath, tmp_buffer, size);
            }
            if(config.logo_path.count && !logo_path)
            {
                ofd_to_c_string(config.logo_path, tmp_buffer);
                
                int size = strlen(tmp_buffer) + 1;
                logo_path = malloc(size);
                memcpy(logo_path, tmp_buffer, size);
            }
            if(config.icon_path.count && !icon_path)
            {
                ofd_to_c_string(config.icon_path, tmp_buffer);
                
                int size = strlen(tmp_buffer) + 1;
                icon_path = malloc(size);
                memcpy(icon_path, tmp_buffer, size);
            }
            if(config.title.count && !title)
            {
                ofd_to_c_string(config.title, tmp_buffer);
                
                int size = strlen(tmp_buffer) + 1;
                title = malloc(size);
                memcpy(title, tmp_buffer, size);
            }
            
            
            ofd_free(config_data.data);
            // Parse the configuration file. END
        }
        
        if(!html_filepath) html_filepath = "result.html";
        
        
        if(!md_files.count)
        {
            printf("Did you forget to specify markdown files to make documentation from?\n");
            return -1;
        }
        
        ofd_b8 status = ofd_generate_documentation(ofd_cast(md_files.data, char**), md_files.count, html_filepath, theme_filepath, logo_path, icon_path, title);
        if(status) printf("Successfully generated '%s'.\n", html_filepath);
        else
        {
            printf("Failed to generate '%s'.\n", html_filepath);
            return -1;
        }
    }
    else
    {
        printf("%s", HELP_MESSAGE);
    }
    
    return 0;
}