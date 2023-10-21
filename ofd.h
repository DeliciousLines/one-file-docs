#ifndef OFD_HEADER
#define OFD_HEADER

#ifdef __cplusplus
extern "C" {
#endif

/*
    -=-=- License: check the end of the file. -=-=-
    
    
    -=-=- How to include this library in your project -=-=-
    You can use this file as a regular header file but you need to #define OFD_IMPLEMENTATION *once* in your codebase before including this
    to bring in the definitions.
    
    
    The goal of this library is very simple: take markdown files and output a single HTML file that combines them all.
    These are the only two calls you will probably ever use:
    
        ofd_generate_documentation()
        ofd_generate_documentation_from_memory()
    
    The first one handles file I/O whereas the second one lets you handle that yourself.
    If you do not need file I/O you can #define OFD_NO_FILE_IO before including this file.
    
    This library uses several utility routines from the C runtime. If you would like to cut dependency on the C runtime you can #define them yourself.
    See the #defines below to see what the library uses from the C runtime. Note that if you do not #define OFD_NO_FILE_IO the library still relies on
    <stdio.h> to read and write files.
    
    
    -=-=- Configuration files -=-=-
    If you want to make your own configuration files with custom fields you can use ofd_parse_fields(). All it needs is a list of fields and text data to parse.
    If you want to use the standard configuration files you can reuse ofd_parse_theme_data() and ofd_parse_configuration_data().
    Note that none of these routines read data from files, that is something you need to handle yourself.
    
    
    -=-=- Syntax highlighting in code blocks -=-=-
    If you want to add a language to the syntax highlighter check ofd_apply_syntax_highlighting(). If the language you want to add happens to be similar to C
    then you can use ofd_apply_c_syntax_highlighting() directly; all you have to do is provide it with a list of keywords and comment indicators.
    In any other case you will have to write your own parser. You can take inspiration from the existing parsers if you want. There are a few basic routines that
    can help you deal with simple elements like string/number literals and keywords.
    
    
    -=-=- Note on speed -=-=-
    Right now the library is not optimised for speed. I do not have relevant speed data so if you find it too slow for your use case feel free to open
    an issue or make a pull request. One thing to note though, you can have an impact on speed if you decide to use a fast allocator for ofd_allocate()
    (we default on malloc() if you do not provide your own allocator).
*/

#ifndef ofd_static
#define ofd_static
#endif


#define ofd_cast(_v, _type) ((_type)(_v))
#define ofd_array_count(_array) (sizeof(_array)/sizeof(_array[0]))
#define ofd_min(_a, _b) (_a < _b? _a : _b)

typedef unsigned char ofd_b8;
typedef unsigned char ofd_u8;
typedef float         ofd_f32;

#define ofd_true  1
#define ofd_false 0


//////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE: you can #define the following yourself if you want to reduce dependency on the C runtime.
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ofd_assert
    #ifdef assert
        #define ofd_assert(_x) assert(_x)
    #else
        #define ofd_assert(_x)
    #endif
#endif


#ifndef ofd_allocate // Make sure you also #define ofd_free() if you use your own allocator.
#include <stdlib.h>
#define ofd_allocate(_size) malloc(_size)
#define ofd_free(_memory)   free(_memory)
#else
    #if !defined(ofd_free)
    #error You must also define ofd_free() when you define ofd_allocate().
    #endif
#endif

#ifndef ofd_strlen
#include <string.h>
#define ofd_strlen(_c_string) strlen(_c_string)
#endif

#ifndef ofd_copy
#include <string.h>
#define ofd_copy(_destination, _source, _size) memcpy(_destination, _source, _size)
#endif

#ifndef ofd_memcmp // This must follow the C runtime memcmp() return value style where 0 means both chunks of memory contain the same data.
#include <string.h>
#define ofd_memcmp(_a, _b, _size) memcmp(_a, _b, _size)
#endif

#ifndef ofd_sprintf // _memory must contain a C string after the operation.
#include <stdio.h>
#define ofd_sprintf(_memory, _format, ...) sprintf(_memory, _format, __VA_ARGS__)
#endif

#ifndef ofd_to_f32
#include <stdlib.h>
#define ofd_to_f32(_c_string) atof(_c_string)
#endif


#ifndef OFD_NO_FILE_IO

#include <stdio.h>

#ifndef ofd_printf
#define ofd_printf(_format, ...) printf(_format, __VA_ARGS__)
#endif

#endif // OFD_NO_FILE_IO


///////////////
// Utility API.

typedef struct
{
    char* data;
    int   count;
} Ofd_String;

#define OFD_STRING(_c_string) {_c_string, sizeof(_c_string) - 1}

ofd_static Ofd_String Ofd_String_(char* c_string);

ofd_static ofd_b8 ofd_string_matches(Ofd_String a, char* b);

ofd_static ofd_b8 ofd_strings_match(Ofd_String a, Ofd_String b);

ofd_static void ofd_to_c_string(Ofd_String s, char* buffer);

ofd_static Ofd_String ofd_get_last_item_from_filepath(Ofd_String filepath);

ofd_static Ofd_String ofd_strip_file_extension(Ofd_String filepath);

ofd_static Ofd_String ofd_cleanup_section_name(Ofd_String name, char* result_memory);

ofd_static Ofd_String ofd_make_section_id(Ofd_String section_name, char* result_memory);


typedef struct
{
    char* data;
    int   item_size;
    int   count;
    int   max_count;
} Ofd_Array;

ofd_static void ofd_array_grow(Ofd_Array* array, int new_count);

ofd_static void ofd_array_init(Ofd_Array* array, int item_size);

ofd_static void ofd_array_init_with_space(Ofd_Array* array, int item_size, int num_items_to_preallocate);

ofd_static void* ofd_array_add_fast(Ofd_Array* array);

ofd_static void* ofd_array_add_fast_multiple(Ofd_Array* array, int count);

ofd_static void ofd_array_add_string(Ofd_Array* destination, Ofd_String source);

ofd_static void ofd_array_add_array(Ofd_Array* destination, Ofd_Array* source);

ofd_static void ofd_free_array(Ofd_Array* array);


#ifndef OFD_NO_FILE_IO
ofd_static ofd_b8 ofd_read_whole_file(char* filepath, Ofd_String* result);

ofd_static ofd_b8 ofd_write_whole_file(char* filepath, void* data, int count);
#endif // OFD_NO_FILE_IO


typedef struct
{
    ofd_f32 r;
    ofd_f32 g;
    ofd_f32 b;
    ofd_f32 a;
} Ofd_Colour;

ofd_static void ofd_colour_to_string(Ofd_Colour colour, char* result);


typedef struct
{
    Ofd_Colour background_colour;
    Ofd_Colour text_colour;
    Ofd_Colour code_block_background_colour;
    Ofd_Colour code_block_text_colour;
    Ofd_Colour link_colour;
    Ofd_Colour image_title_colour;
    Ofd_Colour horizontal_rule_colour;
    Ofd_Colour quote_background_colour;
    Ofd_Colour quote_text_colour;
    Ofd_Colour section_link_text_colour;
    Ofd_Colour section_link_background_colour;
    Ofd_Colour separator_colour;
    Ofd_Colour scrollbar_colour;
    ofd_f32    font_size; // In % of the render target height.
    
    // The following apply to syntax highlighting in code blocks.
    Ofd_Colour syntax_comment_colour;
    Ofd_Colour syntax_string_colour;
    Ofd_Colour syntax_number_colour;
    Ofd_Colour syntax_structure_colour; // e.g. '(', ';', '{', etc.
    Ofd_Colour syntax_keyword_colour;
    Ofd_Colour syntax_directive_colour;
} Ofd_Theme;


#define OFD_MAX_CODE_BLOCK_INDICATOR_COUNT 3
#define OFD_MAX_SECTION_NAME_COUNT         4096
#define OFD_MAX_HEADER_LEVEL               6

typedef struct
{
    int id; // @ Should we get rid of this?
    int hierarchy; // 0 is the highest hierarchy. 1-6 is for markdown headers, 0 means this is a markdown file.
    int name_count;
    int id_count;
    int full_id_count;
    
    char name_memory[OFD_MAX_SECTION_NAME_COUNT];
    char id_memory[OFD_MAX_SECTION_NAME_COUNT];
    char full_id_memory[OFD_MAX_SECTION_NAME_COUNT];
} Ofd_Section;

typedef enum {
    OFD_LESS_THAN_BRACKET    = 0x01,
    OFD_GREATER_THAN_BRACKET = 0x02,
    OFD_LINK                 = 0x04,
} Ofd_Filter_Type;

typedef struct
{
    Ofd_String      text;
    Ofd_Filter_Type type;
} Ofd_Element_To_Filter;

typedef struct
{
    Ofd_String id;            // This is a view.
    Ofd_String link_address;  // This is a view.
    Ofd_String link_title;    // This is a view.
    Ofd_Array additional_css;
} Ofd_Link_Reference;

ofd_static ofd_b8 ofd_filter_plain_text(Ofd_String* text);

ofd_static char* ofd_skip_whitespace(char* c, char* limit);

ofd_static Ofd_String ofd_get_line_view(char* c, char* lower_limit, char* upper_limit);

ofd_static void ofd_log_error(Ofd_Array* log_data, Ofd_String line_view, Ofd_String message);

ofd_static char* ofd_parse_link(char* c, char* limit, Ofd_Array* link_references, Ofd_String* result_name, Ofd_String* result_address, Ofd_String* result_title, Ofd_Array* additional_css, Ofd_Array* log_data);

ofd_static char* ofd_maybe_handle_a_horizontal_rule(char* c, char* limit, char indicator, Ofd_Array* result_html, Ofd_String* text_to_spill);

ofd_static char* ofd_get_next_character_like_in_line(char* c, char* limit, char wanted, char** result_line_end);


///////////////////////////
// Syntax highlighting API.

ofd_static void ofd_syntax_insert_token(Ofd_Array* result_html, Ofd_String token);

ofd_static void ofd_syntax_spill_basic_text(Ofd_Array* result_html, Ofd_String* basic_text, const Ofd_String* keywords, int num_keywords);

ofd_static void ofd_syntax_spill_basic_text_no_keywords(Ofd_Array* result_html, Ofd_String* basic_text);

ofd_static char* ofd_syntax_handle_string(Ofd_Array* result_html, char* c, char* limit);

ofd_static char* ofd_syntax_handle_number(Ofd_Array* result_html, char* c, char* limit);


typedef enum
{
    OFD_SUPPORTS_NESTED_MULTI_LINE_COMMENTS = 0x01,
    OFD_BACKTICK_IS_A_STRING                = 0x02,
    OFD_THIS_IS_RUST                        = 0x02,
} Ofd_Syntax_Flags;

ofd_static void ofd_apply_c_syntax_highlighting(Ofd_Array* result_html, Ofd_String text, Ofd_String single_line_comment_start, Ofd_String multi_line_comment_start, Ofd_String multi_line_comment_end, const Ofd_String* keywords, int num_keywords, Ofd_Syntax_Flags flags);

ofd_static void ofd_apply_syntax_highlighting(Ofd_Array* result_html, Ofd_String text, Ofd_String language);


////////////
// Main API.

ofd_static void ofd_parse_markdown(char* c, char* limit, Ofd_Array* result_html, Ofd_Array* result_sections, Ofd_Array* link_references, int* next_section_id, Ofd_Array* log_data);



typedef enum
{
    OFD_FIELD_COLOUR = 0x01,
    OFD_FIELD_STRING = 0x02,
    OFD_FIELD_F32    = 0x04,
} Ofd_Field_Type;

typedef struct
{
    ///////////////////////////////////////////////////////////////////////
    // NOTE: this struct is to be used in ofd_parse_theme_data() only.
    ///////////////////////////////////////////////////////////////////////
    
    int id; // This must be a flag.
    char* name;
    void* data;
    Ofd_Field_Type type;
} Ofd_Field;

ofd_static ofd_b8 ofd_parse_fields(char* data, int data_size, Ofd_Field* fields, int num_fields, char* error_message);

ofd_static ofd_b8 ofd_parse_theme_data(char* theme_data, int theme_data_size, Ofd_Theme* result, char* error_message);


typedef struct
{
    Ofd_String output_filepath;
    Ofd_String theme_filepath;
    Ofd_String logo_path;
    Ofd_String icon_path;
    Ofd_String title;
} Ofd_Config;

ofd_static ofd_b8 ofd_parse_configuration_data(char* data, int data_size, Ofd_Config* result, char* error_message);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE: you must free output and log_data using ofd_free_arrray().
// You do not need to initialise output and log_data yourself.
// log_data contains a C string describing potential errors.
ofd_static void ofd_generate_documentation_from_memory(Ofd_String* markdown_files, char** markdown_filepaths, int num_markdown_files, Ofd_Array* output, Ofd_Theme theme, char* logo_path, char* icon_path, char* title, Ofd_Array* log_data);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef OFD_NO_FILE_IO
ofd_static ofd_b8 ofd_generate_documentation(char** markdown_filepaths, int num_markdown_files, char* output_filepath, char* theme_filepath, char* logo_path, char* icon_path, char* title);
#endif // OFD_NO_FILE_IO

#ifdef __cplusplus
}
#endif
#endif // OFD_HEADER




#ifdef OFD_IMPLEMENTATION
#ifdef __cplusplus
extern "C" {
#endif

ofd_static Ofd_String Ofd_String_(char* c_string)
{
    Ofd_String result = {c_string, ofd_strlen(c_string)};
    return result;
}

ofd_static ofd_b8 ofd_string_matches(Ofd_String a, char* b)
{
    int b_count = ofd_strlen(b);
    if(a.count != b_count) return ofd_false;
    
    return (ofd_memcmp(a.data, b, b_count) == 0);
}

ofd_static ofd_b8 ofd_strings_match(Ofd_String a, Ofd_String b)
{
    if(a.count != b.count) return ofd_false;
    
    return (ofd_memcmp(a.data, b.data, a.count) == 0);
}

ofd_static void ofd_to_c_string(Ofd_String s, char* buffer)
{
    ofd_copy(buffer, s.data, s.count);
    buffer[s.count] = 0;
}

ofd_static Ofd_String ofd_get_last_item_from_filepath(Ofd_String filepath)
{
    char* c     = filepath.data + filepath.count - 1;
    char* limit = filepath.data;
    while(c >= limit)
    {
        if(*c == '/' || *c == '\\') break; // @ Speed this up;
        c--;
    }
    
    filepath.count = filepath.data + filepath.count - (c + 1);
    filepath.data  = c + 1;
    
    return filepath;
}

ofd_static Ofd_String ofd_strip_file_extension(Ofd_String filepath)
{
    char* c     = filepath.data + filepath.count;
    char* limit = filepath.data;
    while(c >= limit)
    {
        if(*c == '.') // @ Speed this up.
        {
            filepath.count -= filepath.data + filepath.count - c;
            break;
        }
        
        c--;
    }
    
    return filepath;
}

ofd_static Ofd_String ofd_cleanup_section_name(Ofd_String name, char* result_memory)
{
    char* c     = name.data;
    char* limit = name.data + name.count;
    
    int offset = 0;
    
    while(c < limit)
    { // Remove leading whitespace.
        if(*c != ' ' && *c != '\t' && *c != '_') break;
        c++;
    }
    
    Ofd_String s = {c};
    while(c < limit)
    {
        char character = *c;
        
        if(character == '_') // @ Speed this up.
        {
            ofd_copy(result_memory + offset, s.data, s.count);
            offset += s.count;
            
            result_memory[offset] = ' ';
            offset++;
            
            c++;
            s.data  = c;
            s.count = 0;
            
            continue;
        }
        
        if(character == '*' || character == '`') // @ Speed this up.
        {
            ofd_copy(result_memory + offset, s.data, s.count);
            offset += s.count;
            
            c++;
            s.data  = c;
            s.count = 0;
            
            continue;
        }
        
        s.count++;
        c++;
    }
    
    
    ofd_copy(result_memory + offset, s.data, s.count);
    offset += s.count;
    
    name.data  = result_memory;
    name.count = offset;
    
    
    c     = name.data + name.count - 1;
    limit = name.data;
    while(c >= limit)
    { // Remove trailing whitespace.
        if(*c != ' ' && *c != '\t') break;
        
        name.count--;
        c--;
    }
    
    
    return name;
}

ofd_static Ofd_String ofd_make_section_id(Ofd_String section_name, char* result_memory)
{
    /////////////////////////////////////////////////////////////////////////////////////////
    // NOTE: this does not perform cleanups. Use ofd_cleanup_section_name() first to do that.
    /////////////////////////////////////////////////////////////////////////////////////////
    
    char* c     = section_name.data;
    char* limit = section_name.data + section_name.count;
    
    int offset = 0;
    
    Ofd_String s = {c};
    while(c < limit)
    {
        char character = *c;
        if(character == ' ' || character == '\t' || character == '/' || character == '\\')
        {
            ofd_copy(result_memory + offset, s.data, s.count);
            offset += s.count;
            
            while(c < limit)
            { // Replace characters with a '-'.
                character = *c;
                if(character != ' ' && character != '\t' && character != '/' && character != '\\') break;
                c++;
            }
            
            result_memory[offset] = '-';
            offset++;
            
            s.count = 0;
            s.data  = c;
            
            continue;
        }
        
        s.count++;
        c++;
    }
    
    
    ofd_copy(result_memory + offset, s.data, s.count);
    offset += s.count;
    
    
    section_name.data  = result_memory;
    section_name.count = ofd_strlen(result_memory);
    
    return section_name;
}


ofd_static void ofd_array_grow(Ofd_Array* array, int new_count)
{
    ofd_assert(array->max_count < new_count);
    
    int new_max_count = new_count * 2;
    
    int new_size = new_max_count * array->item_size;
    char* new_data = ofd_allocate(new_size);
    
    ofd_copy(new_data, array->data, array->count * array->item_size);
    ofd_free(array->data);
    array->data      = new_data;
    array->max_count = new_max_count;
}

ofd_static void ofd_array_init(Ofd_Array* array, int item_size)
{
    array->data      = NULL;
    array->item_size = item_size;
    array->count     = 0;
    array->max_count = 0;
}

ofd_static void ofd_array_init_with_space(Ofd_Array* array, int item_size, int num_items_to_preallocate)
{
    array->data      = ofd_allocate(num_items_to_preallocate * item_size);
    array->item_size = item_size;
    array->count     = 0;
    array->max_count = num_items_to_preallocate;
}

ofd_static void* ofd_array_add_fast(Ofd_Array* array)
{
    if(array->max_count == array->count) ofd_array_grow(array, array->count + 1);
    
    void* data = array->data + array->count * array->item_size;
    array->count++;
    
    return data;
}

ofd_static void* ofd_array_add_fast_multiple(Ofd_Array* array, int count)
{
    if(array->max_count == array->count) ofd_array_grow(array, array->count + count);
    
    void* data = array->data + array->count * array->item_size;
    array->count += count;
    
    return data;
}

ofd_static void ofd_array_add_string(Ofd_Array* destination, Ofd_String source)
{
    int new_count = destination->count + source.count;
    
    if(destination->max_count < new_count) ofd_array_grow(destination, new_count);
    
    ofd_copy(destination->data + destination->count, source.data, source.count);
    destination->count = new_count;
}

ofd_static void ofd_array_add_array(Ofd_Array* destination, Ofd_Array* source)
{
    ofd_assert(destination->item_size == source->item_size);
    
    int new_count = destination->count + source->count;
    
    if(destination->max_count < new_count) ofd_array_grow(destination, new_count);
    
    ofd_copy(destination->data + destination->count * destination->item_size, source->data, source->count * destination->item_size);
    destination->count = new_count;
}

ofd_static void ofd_free_array(Ofd_Array* array)
{
    if(!array->data) return;
    
    ofd_free(array->data);
    array->max_count = 0;
    array->count     = 0;
    array->data      = NULL;
}


#ifndef OFD_NO_FILE_IO
ofd_static ofd_b8 ofd_read_whole_file(char* filepath, Ofd_String* result)
{
    result->data  = NULL;
    result->count = 0;
    
    ofd_b8 status = ofd_false;
    
    FILE* f = fopen(filepath, "rb");
    if(f)
    {
        status = ofd_true;
        
        fseek(f, 0, SEEK_END);
        result->count = ftell(f);
        rewind(f);
        
        result->data = ofd_allocate(result->count);
        int num_bytes_read = fread(result->data, 1, result->count, f);
        if(num_bytes_read != result->count)
        { // We failed to read the file.
            ofd_free(result->data);
            result->count = 0;
            
            ofd_printf("Failed to correctly read file '%s'.\n", filepath);
            status = ofd_false;
        }
        
        fclose(f);
    }
    else ofd_printf("Failed to open file '%s'.\n", filepath);
    
    return status;
}

ofd_static ofd_b8 ofd_write_whole_file(char* filepath, void* data, int count)
{
    ofd_b8 status = ofd_false;
    
    FILE* f = fopen(filepath, "wb");
    if(f)
    {
        status = ofd_true;
        
        int num_bytes_written = fwrite(data, 1, count, f);
        if(num_bytes_written != count)
        { // We failed to write the file.
            ofd_printf("Failed to correctly write file '%s'.\n", filepath);
            status = ofd_false;
        }
        
        fclose(f);
    }
    else ofd_printf("Failed to create file '%s'.\n", filepath);
    
    return status;
}
#endif // OFD_NO_FILE_IO


ofd_static void ofd_colour_to_string(Ofd_Colour colour, char* result)
{
    if(colour.a < 0.999f)
    {
        ofd_sprintf(result, "#%02x%02x%02x%02x", ofd_cast(colour.r * 255.0f, int), ofd_cast(colour.g * 255.0f, int), ofd_cast(colour.b * 255.0f, int), ofd_cast(colour.a * 255.0f, int));
    }
    else
    {
        ofd_sprintf(result, "#%02x%02x%02x", ofd_cast(colour.r * 255.0f, int), ofd_cast(colour.g * 255.0f, int), ofd_cast(colour.b * 255.0f, int));
    }
}


ofd_static ofd_b8 ofd_filter_plain_text(Ofd_String* text)
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE: we assume the string is just a view into data. This routine returns ofd_true if the string was changed.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    Ofd_Array elements_to_filter;
    ofd_array_init(&elements_to_filter, sizeof(Ofd_Element_To_Filter));
    
    int new_count = text->count;
    
    char* c     = text->data;
    char* limit = text->data + text->count;
    
    while(c < limit)
    {
        if(*c == '<')
        {
            Ofd_Element_To_Filter* element = ofd_array_add_fast(&elements_to_filter);
            element->text.data  = c;
            element->text.count = 1;
            element->type       = OFD_LESS_THAN_BRACKET;
            
            new_count += 3 - 1;
            
            c++;
            continue;
        }
        else if(*c == '>')
        {
            Ofd_Element_To_Filter* element = ofd_array_add_fast(&elements_to_filter);
            element->text.data  = c;
            element->text.count = 1;
            element->type       = OFD_GREATER_THAN_BRACKET;
            
            new_count += 3 - 1;
            
            c++;
            continue;
        }
        
        ofd_b8 is_a_link = ofd_false;
        if(c + 7 <= limit) is_a_link |= (ofd_memcmp(c, "http://",  7) == 0);
        if(c + 8 <= limit) is_a_link |= (ofd_memcmp(c, "https://", 8) == 0);
        
        if(is_a_link)
        {
            Ofd_String link = {c};
            while(c < limit)
            { // Retrieve the full link.
                char character = *c;
                if(character == ' ' || character == '\t' || character == '\n' || character == '\r' || character == '"') break;
                
                link.count++;
                c++;
            }
            
            
            Ofd_Element_To_Filter* element = ofd_array_add_fast(&elements_to_filter);
            element->text = link;
            element->type = OFD_LINK;
            
            new_count += link.count + ofd_strlen("<a class='ofd-external-link' target='_blank' href=''></a>");
            
            continue;
        }
        
        
        c++;
    }
    
    if(!elements_to_filter.count) return ofd_false; // We did not find any elements to filter so we can just return the string unchanged.
    
    
    
    
    char* new_data = ofd_allocate(new_count);
    int offset = 0;
    
    char* sub_text_that_needs_no_filtering_start = text->data;
    
    for(int element_index = 0; element_index < elements_to_filter.count; element_index++)
    { // Build the new text.
        Ofd_Element_To_Filter* element = ofd_cast(elements_to_filter.data, Ofd_Element_To_Filter*) + element_index;
        
        
        // Copy unfiltered text data.
        int sub_text_that_needs_no_filtering_count = element->text.data - sub_text_that_needs_no_filtering_start;
        ofd_copy(new_data + offset, sub_text_that_needs_no_filtering_start, sub_text_that_needs_no_filtering_count);
        offset += sub_text_that_needs_no_filtering_count;
        
        sub_text_that_needs_no_filtering_start = element->text.data + element->text.count;
        /////////////////////////////
        
        
        switch(element->type)
        {
            case OFD_LESS_THAN_BRACKET:
            {
                ofd_copy(new_data + offset, "&lt", 3);
                offset += 3;
            } break;
            
            case OFD_GREATER_THAN_BRACKET:
            {
                ofd_copy(new_data + offset, "&gt", 3);
                offset += 3;
            } break;
            
            case OFD_LINK:
            {
                Ofd_String text_to_insert = Ofd_String_("<a class='ofd-external-link' target='_blank' href='");
                ofd_copy(new_data + offset, text_to_insert.data, text_to_insert.count);
                offset += text_to_insert.count;
                
                ofd_copy(new_data + offset, element->text.data, element->text.count);
                offset += element->text.count;
                
                text_to_insert = Ofd_String_("'>");
                ofd_copy(new_data + offset, text_to_insert.data, text_to_insert.count);
                offset += text_to_insert.count;
                
                ofd_copy(new_data + offset, element->text.data, element->text.count);
                offset += element->text.count;
                
                text_to_insert = Ofd_String_("</a>");
                ofd_copy(new_data + offset, text_to_insert.data, text_to_insert.count);
                offset += text_to_insert.count;
            } break;
        }
    }
    
    // Copy the remaining unfiltered data.
    int sub_text_that_needs_no_filtering_count = limit - sub_text_that_needs_no_filtering_start;
    ofd_copy(new_data + offset, sub_text_that_needs_no_filtering_start, sub_text_that_needs_no_filtering_count);
    offset += sub_text_that_needs_no_filtering_count;
    //////////////////////////////////////
    
    
    text->data  = new_data;
    text->count = offset;
    
    ofd_free_array(&elements_to_filter);
    
    return ofd_true;
}

ofd_static char* ofd_skip_whitespace(char* c, char* limit)
{
    while(c < limit)
    {
        char character = *c;
        if(character != ' ' && character != '\t') return c;
        c++;
    }
    
    return c;
}

ofd_static Ofd_String ofd_get_line_view(char* c, char* lower_limit, char* upper_limit)
{
    while(c >= lower_limit)
    {
        if(*c == '\n' || *c == '\r') break;
        c--;
    }
    c++;
    
    Ofd_String result = {c};
    while(c < upper_limit)
    {
        if(*c == '\n' || *c == '\r') break;
        c++;
    }
    
    result.count = c - result.data;
    return result;
}

ofd_static void ofd_log_error(Ofd_Array* log_data, Ofd_String line_view, Ofd_String message)
{
    ofd_array_add_string(log_data, Ofd_String_("*** Error ***\n"));
    ofd_array_add_string(log_data, line_view);
    ofd_array_add_string(log_data, Ofd_String_("\n\n"));
    ofd_array_add_string(log_data, message);
    ofd_array_add_string(log_data, Ofd_String_("\n\n"));
}

ofd_static char* ofd_parse_link(char* c, char* limit, Ofd_Array* link_references, Ofd_String* result_name, Ofd_String* result_address, Ofd_String* result_title, Ofd_Array* additional_css, Ofd_Array* log_data)
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE: c should point to the first '['. If this turns out to no be an actual link, this routine returns NULL; Otherwise it returns the first character
    // to come after the link element.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    char* lower_limit = c;
    
    ofd_array_init(additional_css, 1);
    
    c++;
    result_name->data  = c;
    result_name->count = 0;
    
    int nesting_level = 1;
    
    while(c < limit)
    {
        if(*c == '[')
        {
            nesting_level++;
        }
        else if(*c == ']')
        {
            nesting_level--;
            if(!nesting_level)
            {
                c++;
                break;
            }
        }
        
        result_name->count++;
        c++;
    }
    
    c = ofd_skip_whitespace(c, limit);
    if(c == limit) return NULL; // This does not appear to be a link after all.
    
    
    if(*c == '[')
    {
        // This link uses a reference. START
        // @ We should log errors!!!
        c++;
        Ofd_String ref_id = {c};
        while(c < limit)
        {
            char character = *c;
            if(character == ']') break;
            if(character == '\n' || character == '\r')
            {
                Ofd_String line_view = ofd_get_line_view(c, lower_limit, limit);
                ofd_log_error(log_data, line_view, Ofd_String_("This does not look like a valid link reference."));
                return NULL; // This does not look like a valid link reference.
            }
            
            c++;
        }
        
        ref_id.count = c - ref_id.data;
        c++; // Skip ']'.
        
        Ofd_Link_Reference* reference = NULL;
        for(int i = 0; i < link_references->count; i++)
        {
            Ofd_Link_Reference* ref = ofd_cast(link_references->data, Ofd_Link_Reference*) + i;
            if(ofd_strings_match(ref->id, ref_id))
            {
                reference = ref;
                break;
            }
        }
        
        if(!reference)
        {
            Ofd_String line_view = ofd_get_line_view(c, lower_limit, limit);
            ofd_log_error(log_data, line_view, Ofd_String_("The reference to this link could no be found. Make sure to declare a reference *before* using it."));
            return NULL; // The reference for this link could not be found.
        }
        
        *result_address = reference->link_address;
        *result_title   = reference->link_title;
        
        if(reference->additional_css.count)
        {
            char* css = ofd_array_add_fast_multiple(additional_css, reference->additional_css.count);
            ofd_copy(css, reference->additional_css.data, reference->additional_css.count);
        }
        // This link uses a reference. END
        
        return c;
    }
    
    
    if(*c == '(')
    { // This is definitely a link.
        c++;
        
        result_address->data  = c;
        result_address->count = 0;
        result_title->count   = 0;
        
        while(c < limit)
        { // Retrieve the link address.
            char character = *c;
            if(character == ' ' || character == '\t' || character == ')') break;
            
            result_address->count++;
            c++; 
        }
        
        if(c < limit && *c != ')')
        { // There is more to parse.
            while(c < limit)
            {
                c = ofd_skip_whitespace(c, limit);
                if(c == limit) break;
                
                char character = *c;
                
                if(character == '"')
                {
                    // Retrieve a title. START
                    c++;
                    result_title->data  = c;
                    result_title->count = 0;
                    while(c < limit)
                    { // Get the link title.
                        if(*c == '"')
                        {
                            c++;
                            break;
                        }
                        
                        result_title->count++;
                        c++;
                    }
                    // Retrieve a title. END
                    
                    continue;
                }
                
                
                if(character == 'w' || character == 'h' || character == 'r')
                {
                    // Parse additional CSS. START
                    if(c + 1 < limit && c[1] == '=') c++;
                    if(c + 1 < limit && c[1] == '"')
                    {
                        c += 2;
                        
                        Ofd_String css = {c};
                        while(c < limit)
                        {
                            if(*c == '"')
                            {
                                c++;
                                break;
                            }
                            
                            css.count++;
                            c++;
                        }
                        
                        if     (character == 'w') ofd_array_add_string(additional_css, Ofd_String_("width: "));
                        else if(character == 'h') ofd_array_add_string(additional_css, Ofd_String_("height: "));
                        else if(character == 'r') ofd_array_add_string(additional_css, Ofd_String_("border-radius: "));
                        
                        ofd_array_add_string(additional_css, css);
                        ofd_array_add_string(additional_css, Ofd_String_(";"));
                        
                        continue;
                    }
                    // Parse additional CSS. END
                }
                
                
                if(character == ')')
                { // This is the end of the link.
                    c++;
                    break;
                }
                
                c++;
            }
        }
        else c += (c != limit);
        
        
        return c;
    }
    
    
    return NULL;
}

ofd_static char* ofd_maybe_handle_a_horizontal_rule(char* c, char* limit, char indicator, Ofd_Array* result_html, Ofd_String* text_to_spill)
{
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE: if this does not turn out to be a horizontal rule this routine returns NULL, otherwise it returns the next character to parse.
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    int num_indicators_in_a_row = 0;
    while(c < limit)
    {
        if(*c != indicator) break;
        
        num_indicators_in_a_row++;
        c++;
    }
    
    if(num_indicators_in_a_row >= 3 && (c == limit || *c == '\n' || *c == '\r'))
    {
        ofd_array_add_string(result_html, *text_to_spill);
        text_to_spill->count = 0;
        
        ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-horizontal-rule'></div>"));
        
        if(c < limit)
        { // Go to the next line.
            if(*c == '\n') c++;
            if(c < limit && *c == '\r') c++;
        }
        
        return c;
    }
    
    
    return NULL;
}

ofd_static char* ofd_get_next_character_like_in_line(char* c, char* limit, char wanted, char** result_line_end)
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE: if a suitable character is found this routine returns the address at which the character was found, otherwise it returns NULL. If the end of
    // the line is reached result_line_end takes its address.
    // We skip code blocks and escaped characters.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    *result_line_end = NULL;
    
    ofd_b8 ignore_next_character  = ofd_false;
    char* code_block_start = NULL;
    int code_block_start_count = 0;
    
    while(c < limit)
    {
        char character = *c;
        if(character == '\n' || character == '\r')
        {
            *result_line_end = c;
            break; // We reached the end of the line.
        }
        
        if(!ignore_next_character)
        {
            if(character == '\\') ignore_next_character = (code_block_start == NULL);
            else if(character == '`')
            {
                if(code_block_start)
                { // Check if we reached the end of a code block.
                    if(c + code_block_start_count <= limit)
                    {
                        char* tmp_c = c;
                        while(tmp_c < limit)
                        {
                            if(*tmp_c != '`') break;
                            tmp_c++;
                        }
                        
                        int num_backticks = tmp_c - c;
                        if(num_backticks == code_block_start_count)
                        {
                            c += code_block_start_count;
                            
                            code_block_start       = NULL;
                            code_block_start_count = 0;
                            continue; // We reached the end of a code block.
                        }
                        
                        c += num_backticks;
                        continue;
                    }
                }
                else
                {
                    // Handle a code block start. START
                    code_block_start       = c;
                    code_block_start_count = 0;
                    while(c < limit && code_block_start_count < OFD_MAX_CODE_BLOCK_INDICATOR_COUNT)
                    {
                        if(*c != '`') break;
                        
                        code_block_start_count++;
                        c++;
                    }
                    // Handle a code block start. END
                    
                    continue;
                }
            }
            else if(character == wanted && !code_block_start) return c;
        }
        else ignore_next_character = ofd_false;
        
        c++;
    }
    
    return NULL;
}


ofd_static void ofd_syntax_insert_token(Ofd_Array* result_html, Ofd_String token)
{
    ofd_b8 got_filtered = ofd_filter_plain_text(&token);
    ofd_array_add_string(result_html, token);
    if(got_filtered) ofd_free(token.data);
}

ofd_static void ofd_syntax_spill_basic_text(Ofd_Array* result_html, Ofd_String* basic_text, const Ofd_String* keywords, int num_keywords)
{
    ofd_b8 got_a_keyword = ofd_false;
    for(int i = 0; i < num_keywords; i++)
    {
        if(ofd_strings_match(keywords[i], *basic_text))
        {
            got_a_keyword = ofd_true;
            
            ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-syntax ofd-syntax-keyword'>"));
            ofd_syntax_insert_token(result_html, *basic_text);
            ofd_array_add_string(result_html, Ofd_String_("</div>"));
            
            basic_text->count = 0;
            return;
        }
    }
    
    if(!got_a_keyword)
    {
        ofd_syntax_insert_token(result_html, *basic_text);
        basic_text->count = 0;
    }
}

ofd_static void ofd_syntax_spill_basic_text_no_keywords(Ofd_Array* result_html, Ofd_String* basic_text)
{
    ofd_syntax_insert_token(result_html, *basic_text);
    basic_text->count = 0;
}

ofd_static char* ofd_syntax_handle_string(Ofd_Array* result_html, char* c, char* limit)
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE: we expect c to start on the first delimiter character of the string, e.g. on the first '"'.
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    char delimiter = *c;
    Ofd_String token = {c};
    c++;
    
    ofd_b8 is_escaped = ofd_false;
    while(c < limit)
    {
        if(is_escaped)
        {
            is_escaped = ofd_false;
            c++;
            continue;
        }
        
        if(*c == '\\') is_escaped = ofd_true;
        else if(*c == delimiter)
        {
            c++;
            break;
        }
        
        c++;
    }
    
    token.count = c - token.data;
    
    ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-syntax ofd-syntax-string'>"));
    ofd_syntax_insert_token(result_html, token);
    ofd_array_add_string(result_html, Ofd_String_("</div>"));
    
    return c;
}

ofd_static char* ofd_syntax_handle_number(Ofd_Array* result_html, char* c, char* limit)
{
    Ofd_String token = {c};
    while(c < limit)
    {
        char character = *c;
        
        if(
            character != '.' &&
            character != '_' &&
            !(character >= '0' && character <= '9') &&
            !(character >= 'a' && character <= 'z') &&
            !(character >= 'A' && character <= 'Z')
        )
        {
            break;
        }
        
        c++;
    }
    
    
    token.count = c - token.data;
    
    ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-syntax ofd-syntax-number'>"));
    ofd_syntax_insert_token(result_html, token);
    ofd_array_add_string(result_html, Ofd_String_("</div>"));
    
    return c;
}


ofd_static void ofd_apply_c_syntax_highlighting(Ofd_Array* result_html, Ofd_String text, Ofd_String single_line_comment_start, Ofd_String multi_line_comment_start, Ofd_String multi_line_comment_end, const Ofd_String* keywords, int num_keywords, Ofd_Syntax_Flags flags)
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE: if the language does not support a certain type of commenting style just set the corresponding comment indicator to a blank value, i.e. "".
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    ofd_b8 supports_nested_multiline_comments = (flags & OFD_SUPPORTS_NESTED_MULTI_LINE_COMMENTS) != 0;
    
    char* c           = text.data;
    char* limit       = text.data + text.count;
    char* lower_limit = text.data;
    
    Ofd_String basic_text        = {c};
    Ofd_String current_directive = {0};
    
    
    while(c < limit)
    {
        if(single_line_comment_start.count && c + single_line_comment_start.count <= limit && ofd_memcmp(c, single_line_comment_start.data, single_line_comment_start.count) == 0)
        {
            // Handle a single line comment. START
            ofd_syntax_spill_basic_text(result_html, &basic_text, keywords, num_keywords);
            
            Ofd_String token = {c};
            c += single_line_comment_start.count;
            while(c < limit)
            {
                if(*c == '\n' || *c == '\r') break;
                c++;
            }
            
            token.count = c - token.data;
            
            ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-syntax ofd-syntax-comment'>"));
            ofd_syntax_insert_token(result_html, token);
            ofd_array_add_string(result_html, Ofd_String_("</div>"));
            // Handle a single line comment. END
            
            continue;
        }
        
        if(multi_line_comment_start.count && c + multi_line_comment_start.count <= limit && ofd_memcmp(c, multi_line_comment_start.data, multi_line_comment_start.count) == 0)
        {
            // Handle a multi-line comment. START
            ofd_syntax_spill_basic_text(result_html, &basic_text, keywords, num_keywords);
            
            int imbrication_level = 1;
            
            Ofd_String token = {c};
            c += multi_line_comment_start.count;
            while(c < limit)
            {
                if(c + multi_line_comment_end.count <= limit && ofd_memcmp(c, multi_line_comment_end.data, multi_line_comment_end.count) == 0)
                { // Check for the end of the comment.
                    c += multi_line_comment_end.count;
                    imbrication_level--;
                    if(!imbrication_level) break; // This is the end of the multi-line comment.
                    
                    continue;
                }
                
                if(supports_nested_multiline_comments)
                {
                    if(c + multi_line_comment_start.count <= limit && ofd_memcmp(c, multi_line_comment_start.data, multi_line_comment_start.count) == 0)
                    { // Check for a nested multi-line comment.
                        c += multi_line_comment_start.count;
                        imbrication_level++;
                        continue;
                    }
                }
                
                
                char character = *c;
                switch(character)
                {
                    case '"':
                    {
                        // Skip a string. START
                        c++;
                        
                        ofd_b8 is_escaped = ofd_false;
                        while(c < limit)
                        {
                            if(is_escaped)
                            {
                                is_escaped = ofd_false;
                                c++;
                                continue;
                            }
                            
                            if(*c == '\\') is_escaped = ofd_true;
                            else if(*c == '"')
                            {
                                c++;
                                break; // We reached the end of the string.
                            }
                            
                            c++;
                        }
                        // Skip a string. END
                    } break;
                    
                    default:
                    {
                        c++;
                    } break;
                }
            }
            
            token.count = c - token.data;
            
            ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-syntax ofd-syntax-comment'>"));
            ofd_syntax_insert_token(result_html, token);
            ofd_array_add_string(result_html, Ofd_String_("</div>"));
            // Handle a multi-line comment. END
            
            continue;
        }
        
        
        char character = *c;
        switch(character)
        {
            case '{':
            case '}':
            case '(':
            case ')':
            case '[':
            case ']':
            case '?':
            case ':':
            case ';':
            {
                ofd_syntax_spill_basic_text(result_html, &basic_text, keywords, num_keywords);
                
                Ofd_String token = {c};
                while(c < limit)
                {
                    if(*c != character) break;
                    c++;
                }
                
                token.count = c - token.data;
                
                ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-syntax ofd-syntax-structure'>"));
                ofd_syntax_insert_token(result_html, token);
                ofd_array_add_string(result_html, Ofd_String_("</div>"));
            } break;
            
            
            case '#':
            {
                // Handle a compiler directive. START
                ofd_syntax_spill_basic_text(result_html, &basic_text, keywords, num_keywords);
                
                Ofd_String token = {c};
                while(c < limit)
                {
                    character = *c;
                    if(character == ' ' || character == '\t' || character == '\n' || character == '\r' || character == '(' || character == '[' || character == '{')
                    {
                        break;
                    }
                    
                    token.count++;
                    c++;
                }
                
                current_directive = token;
                
                ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-syntax ofd-syntax-directive'>"));
                ofd_syntax_insert_token(result_html, token);
                ofd_array_add_string(result_html, Ofd_String_("</div>"));
                // Handle a compiler directive. END
            } break;
            
            
            case '<':
            {
                ofd_syntax_spill_basic_text(result_html, &basic_text, keywords, num_keywords);
                
                if(ofd_string_matches(current_directive, "#include"))
                {
                    // Handle a filepath for an #include directive. START
                    Ofd_String token = {c};
                    while(c < limit)
                    {
                        character = *c;
                        if(character == '>')
                        {
                            token.count++;
                            c++;
                            break;
                        }
                        if(character == '\n' || character == '\r') break;
                        
                        token.count++;
                        c++;
                    }
                    
                    ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-syntax ofd-syntax-string'>"));
                    ofd_syntax_insert_token(result_html, token);
                    ofd_array_add_string(result_html, Ofd_String_("</div>"));
                    // Handle a filepath for an #include directive. END
                    
                    break;
                }
                
                goto do_the_default_thing;
            } break;
            
            
            case '\n':
            case '\r':
            {
                // Handle new lines. START
                ofd_syntax_spill_basic_text(result_html, &basic_text, keywords, num_keywords);
                
                Ofd_String token = {c};
                while(c < limit)
                {
                    if(*c != '\r' && *c != '\n') break;
                    c++;
                }
                
                token.count = c - token.data;
                ofd_array_add_string(result_html, token);
                
                current_directive.count = 0;
                // Handle new lines. END
            } break;
            
            case ' ':
            case '\t':
            {
                // Handle whitespace. START
                ofd_syntax_spill_basic_text(result_html, &basic_text, keywords, num_keywords);
                
                Ofd_String token = {c};
                while(c < limit)
                {
                    if(*c != ' ' && *c != '\t') break;
                    c++;
                }
                
                token.count = c - token.data;
                ofd_array_add_string(result_html, token);
                // Handle whitespace. END
            } break;
            
            
            case '"':
            case '\'':
            {
                ofd_syntax_spill_basic_text(result_html, &basic_text, keywords, num_keywords);
                
                int static_keyword_count = ofd_strlen("'static");
                
                if((flags & OFD_THIS_IS_RUST) && c + static_keyword_count <= limit)
                { // 'static is an actual keyword in Rust so we need to account for that.
                    if(ofd_memcmp(c, "'static", static_keyword_count) == 0)
                    {
                        ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-syntax ofd-syntax-keyword'>'static</div>"));
                        c += static_keyword_count;
                        break;
                    }
                }
                
                c = ofd_syntax_handle_string(result_html, c, limit);
            } break;
            
            case '`':
            {
                ofd_syntax_spill_basic_text(result_html, &basic_text, keywords, num_keywords);
                
                if(flags & OFD_BACKTICK_IS_A_STRING) c = ofd_syntax_handle_string(result_html, c, limit);
                else goto do_the_default_thing;
            } break;
            
            
            case '=':
            case '>':
            case '+':
            case '-':
            case '*':
            case '/':
            case '%':
            case '^':
            case '$':
            case '!':
            case ',':
            case '|':
            case '&':
            case '~':
            case '@':
            case '\\':
            {
                if(basic_text.count)
                {
                    for(int i = 0; i < basic_text.count; i++)
                    {
                        if(basic_text.data[i] == character) goto do_the_default_thing; // This is already a special character so no need to spill it.
                    }
                    
                    ofd_syntax_spill_basic_text(result_html, &basic_text, keywords, num_keywords);
                }
                
                goto do_the_default_thing;
            } break;
            
            
            default:
            {
                do_the_default_thing:;
                
                ofd_b8 this_is_a_number = (character >= '0' && character <= '9');
                if(character == '.' && c + 1 < limit && (c[1] >= '0' && c[1] <= '9')) this_is_a_number = ofd_true;
                if(c != lower_limit)
                {
                    char previous_char = *(c - 1);
                    if((previous_char >= 'a' && previous_char <= 'z') || (previous_char >= 'A' && previous_char <= 'Z')) this_is_a_number = ofd_false;
                }
                
                if(this_is_a_number)
                { // Handle a number.
                    ofd_syntax_spill_basic_text(result_html, &basic_text, keywords, num_keywords);
                    c = ofd_syntax_handle_number(result_html, c, limit);
                    
                    break;
                }
                
                
                if(!basic_text.count) basic_text.data = c;
                basic_text.count++;
                c++;
            } break;
        }
    }
    
    ofd_syntax_spill_basic_text(result_html, &basic_text, keywords, num_keywords);
}

ofd_static void ofd_apply_syntax_highlighting(Ofd_Array* result_html, Ofd_String text, Ofd_String language)
{
    if(ofd_string_matches(language, "8"))
    {
        const Ofd_String bytes_keywords[] = {
            OFD_STRING("b8"),
            OFD_STRING("break"),
            OFD_STRING("cast"),
            OFD_STRING("Code"),
            OFD_STRING("continue"),
            OFD_STRING("f32"),
            OFD_STRING("false"),
            OFD_STRING("for"),
            OFD_STRING("else"),
            OFD_STRING("enum"),
            OFD_STRING("enum_flags"),
            OFD_STRING("if"),
            OFD_STRING("return"),
            OFD_STRING("s8"),
            OFD_STRING("s16"),
            OFD_STRING("s32"),
            OFD_STRING("s64"),
            OFD_STRING("size_of"),
            OFD_STRING("String"),
            OFD_STRING("struct"),
            OFD_STRING("switch"),
            OFD_STRING("true"),
            OFD_STRING("Type"),
            OFD_STRING("type_of"),
            OFD_STRING("u8"),
            OFD_STRING("u16"),
            OFD_STRING("u32"),
            OFD_STRING("u64"),
            OFD_STRING("while"),
        };
        
        ofd_apply_c_syntax_highlighting(result_html, text, Ofd_String_("//"), Ofd_String_("/*"), Ofd_String_("*/"), bytes_keywords, ofd_array_count(bytes_keywords), OFD_SUPPORTS_NESTED_MULTI_LINE_COMMENTS);
    }
    else if(ofd_string_matches(language, "jai"))
    {
        const Ofd_String jai_keywords[] = {
            OFD_STRING("Any"),
            OFD_STRING("asm"),
            OFD_STRING("bool"),
            OFD_STRING("break"),
            OFD_STRING("case"),
            OFD_STRING("cast"),
            OFD_STRING("Code"),
            OFD_STRING("continue"),
            OFD_STRING("defer"),
            OFD_STRING("false"),
            OFD_STRING("float"),
            OFD_STRING("float32"),
            OFD_STRING("float64"),
            OFD_STRING("for"),
            OFD_STRING("else"),
            OFD_STRING("enum"),
            OFD_STRING("enum_flags"),
            OFD_STRING("if"),
            OFD_STRING("inline"),
            OFD_STRING("int"),
            OFD_STRING("return"),
            OFD_STRING("s8"),
            OFD_STRING("s16"),
            OFD_STRING("s32"),
            OFD_STRING("s64"),
            OFD_STRING("size_of"),
            OFD_STRING("string"),
            OFD_STRING("struct"),
            OFD_STRING("true"),
            OFD_STRING("type_of"),
            OFD_STRING("u8"),
            OFD_STRING("u16"),
            OFD_STRING("u32"),
            OFD_STRING("u64"),
            OFD_STRING("using"),
            OFD_STRING("void"),
            OFD_STRING("while"),
            OFD_STRING("xx"),
        };
        
        ofd_apply_c_syntax_highlighting(result_html, text, Ofd_String_("//"), Ofd_String_("/*"), Ofd_String_("*/"), jai_keywords, ofd_array_count(jai_keywords), OFD_SUPPORTS_NESTED_MULTI_LINE_COMMENTS);
    }
    else if(ofd_string_matches(language, "c") || ofd_string_matches(language, "cpp") || ofd_string_matches(language, "c++"))
    {
        const Ofd_String c_keywords[] = { // Taken from https://en.cppreference.com/w/cpp/keyword
            OFD_STRING("alignas"),
            OFD_STRING("alignof"),
            OFD_STRING("and"),
            OFD_STRING("and_eq"),
            OFD_STRING("asm"),
            OFD_STRING("atomic_cancel"),
            OFD_STRING("atomic_commit"),
            OFD_STRING("atomic_noexcept"),
            OFD_STRING("auto"),
            OFD_STRING("bitand"),
            OFD_STRING("bitor"),
            OFD_STRING("bool"),
            OFD_STRING("break"),
            OFD_STRING("case"),
            OFD_STRING("catch"),
            OFD_STRING("char"),
            OFD_STRING("char_8t"),
            OFD_STRING("char_16t"),
            OFD_STRING("char_32t"),
            OFD_STRING("class"),
            OFD_STRING("compl"),
            OFD_STRING("concept"),
            OFD_STRING("const"),
            OFD_STRING("consteval"),
            OFD_STRING("constexpr"),
            OFD_STRING("constinit"),
            OFD_STRING("const_cast"),
            OFD_STRING("continue"),
            OFD_STRING("co_await"),
            OFD_STRING("co_return"),
            OFD_STRING("co_yield"),
            OFD_STRING("decltype"),
            OFD_STRING("default"),
            OFD_STRING("delete"),
            OFD_STRING("do"),
            OFD_STRING("double"),
            OFD_STRING("dynamic_cast"),
            OFD_STRING("else"),
            OFD_STRING("enum"),
            OFD_STRING("explicit"),
            OFD_STRING("export"),
            OFD_STRING("extern"),
            OFD_STRING("false"),
            OFD_STRING("float"),
            OFD_STRING("for"),
            OFD_STRING("friend"),
            OFD_STRING("goto"),
            OFD_STRING("if"),
            OFD_STRING("inline"),
            OFD_STRING("int"),
            OFD_STRING("long"),
            OFD_STRING("mutable"),
            OFD_STRING("namespace"),
            OFD_STRING("new"),
            OFD_STRING("noexcept"),
            OFD_STRING("not"),
            OFD_STRING("not_eq"),
            OFD_STRING("nullptr"),
            OFD_STRING("operator"),
            OFD_STRING("or"),
            OFD_STRING("or_eq"),
            OFD_STRING("private"),
            OFD_STRING("protected"),
            OFD_STRING("public"),
            OFD_STRING("reflexpr"),
            OFD_STRING("register"),
            OFD_STRING("reinterpret_cast"),
            OFD_STRING("requires"),
            OFD_STRING("return"),
            OFD_STRING("short"),
            OFD_STRING("signed"),
            OFD_STRING("sizeof"),
            OFD_STRING("static"),
            OFD_STRING("static_assert"),
            OFD_STRING("static_cast"),
            OFD_STRING("struct"),
            OFD_STRING("switch"),
            OFD_STRING("synchronised"),
            OFD_STRING("template"),
            OFD_STRING("this"),
            OFD_STRING("thread_local"),
            OFD_STRING("throw"),
            OFD_STRING("true"),
            OFD_STRING("try"),
            OFD_STRING("typedef"),
            OFD_STRING("typeid"),
            OFD_STRING("typename"),
            OFD_STRING("union"),
            OFD_STRING("unsigned"),
            OFD_STRING("using"),
            OFD_STRING("virtual"),
            OFD_STRING("void"),
            OFD_STRING("volatile"),
            OFD_STRING("wchar_t"),
            OFD_STRING("while"),
            OFD_STRING("xor"),
            OFD_STRING("xor_eq"),
        };
        
        ofd_apply_c_syntax_highlighting(result_html, text, Ofd_String_("//"), Ofd_String_("/*"), Ofd_String_("*/"), c_keywords, ofd_array_count(c_keywords), 0);
    }
    else if(ofd_string_matches(language, "go"))
    {
        const Ofd_String go_keywords[] = { // Taken from https://go.dev/ref/spec
            OFD_STRING("break"),
            OFD_STRING("default"),
            OFD_STRING("func"),
            OFD_STRING("interface"),
            OFD_STRING("select"),
            OFD_STRING("case"),
            OFD_STRING("defer"),
            OFD_STRING("go"),
            OFD_STRING("map"),
            OFD_STRING("struct"),
            OFD_STRING("chan"),
            OFD_STRING("else"),
            OFD_STRING("goto"),
            OFD_STRING("package"),
            OFD_STRING("switch"),
            OFD_STRING("const"),
            OFD_STRING("fallthrough"),
            OFD_STRING("if"),
            OFD_STRING("range"),
            OFD_STRING("type"),
            OFD_STRING("continue"),
            OFD_STRING("for"),
            OFD_STRING("import"),
            OFD_STRING("return"),
            OFD_STRING("var"),
        };
        
        ofd_apply_c_syntax_highlighting(result_html, text, Ofd_String_("//"), Ofd_String_("/*"), Ofd_String_("*/"), go_keywords, ofd_array_count(go_keywords), OFD_BACKTICK_IS_A_STRING);
    }
    else if(ofd_string_matches(language, "rs") || ofd_string_matches(language, "rust"))
    {
        const Ofd_String rust_keywords[] = { // Taken from https://doc.rust-lang.org/reference/keywords.html
            OFD_STRING("as"),
            OFD_STRING("break"),
            OFD_STRING("const"),
            OFD_STRING("continue"),
            OFD_STRING("crate"),
            OFD_STRING("else"),
            OFD_STRING("enum"),
            OFD_STRING("extern"),
            OFD_STRING("false"),
            OFD_STRING("fn"),
            OFD_STRING("for"),
            OFD_STRING("if"),
            OFD_STRING("impl"),
            OFD_STRING("in"),
            OFD_STRING("let"),
            OFD_STRING("loop"),
            OFD_STRING("match"),
            OFD_STRING("mod"),
            OFD_STRING("move"),
            OFD_STRING("mut"),
            OFD_STRING("pub"),
            OFD_STRING("ref"),
            OFD_STRING("return"),
            OFD_STRING("self"),
            OFD_STRING("Self"),
            OFD_STRING("static"),
            OFD_STRING("struct"),
            OFD_STRING("super"),
            OFD_STRING("trait"),
            OFD_STRING("true"),
            OFD_STRING("type"),
            OFD_STRING("unsafe"),
            OFD_STRING("use"),
            OFD_STRING("where"),
            OFD_STRING("while"),
            OFD_STRING("async"),
            OFD_STRING("await"),
            OFD_STRING("dyn"),
            OFD_STRING("macro_rules"),
            OFD_STRING("union"),
        };
        
        ofd_apply_c_syntax_highlighting(result_html, text, Ofd_String_("//"), Ofd_String_("/*"), Ofd_String_("*/"), rust_keywords, ofd_array_count(rust_keywords), OFD_THIS_IS_RUST | OFD_SUPPORTS_NESTED_MULTI_LINE_COMMENTS);
    }
    else if(ofd_string_matches(language, "cs") || ofd_string_matches(language, "c#"))
    {
        const Ofd_String cs_keywords[] = { // Taken from https://learn.microsoft.com/en-us/dotnet/csharp/language-reference/keywords/
            OFD_STRING("abstract"),
            OFD_STRING("as"),
            OFD_STRING("base"),
            OFD_STRING("bool"),
            OFD_STRING("break"),
            OFD_STRING("byte"),
            OFD_STRING("case"),
            OFD_STRING("catch"),
            OFD_STRING("char"),
            OFD_STRING("checked"),
            OFD_STRING("class"),
            OFD_STRING("const"),
            OFD_STRING("continue"),
            OFD_STRING("decimal"),
            OFD_STRING("default"),
            OFD_STRING("delegate"),
            OFD_STRING("do"),
            OFD_STRING("double"),
            OFD_STRING("else"),
            OFD_STRING("enum"),
            OFD_STRING("event"),
            OFD_STRING("explicit"),
            OFD_STRING("extern"),
            OFD_STRING("false"),
            OFD_STRING("finally"),
            OFD_STRING("fixed"),
            OFD_STRING("float"),
            OFD_STRING("for"),
            OFD_STRING("foreach"),
            OFD_STRING("goto"),
            OFD_STRING("if"),
            OFD_STRING("implicit"),
            OFD_STRING("in"),
            OFD_STRING("int"),
            OFD_STRING("interface"),
            OFD_STRING("internal"),
            OFD_STRING("is"),
            OFD_STRING("lock"),
            OFD_STRING("long"),
            OFD_STRING("namespace"),
            OFD_STRING("new"),
            OFD_STRING("null"),
            OFD_STRING("object"),
            OFD_STRING("operator"),
            OFD_STRING("out"),
            OFD_STRING("override"),
            OFD_STRING("params"),
            OFD_STRING("private"),
            OFD_STRING("protected"),
            OFD_STRING("public"),
            OFD_STRING("readonly"),
            OFD_STRING("ref"),
            OFD_STRING("return"),
            OFD_STRING("sbyte"),
            OFD_STRING("sealed"),
            OFD_STRING("short"),
            OFD_STRING("sizeof"),
            OFD_STRING("stackalloc"),
            OFD_STRING("static"),
            OFD_STRING("string"),
            OFD_STRING("struct"),
            OFD_STRING("switch"),
            OFD_STRING("this"),
            OFD_STRING("throw"),
            OFD_STRING("true"),
            OFD_STRING("try"),
            OFD_STRING("typeof"),
            OFD_STRING("uint"),
            OFD_STRING("ulong"),
            OFD_STRING("unchecked"),
            OFD_STRING("unsafe"),
            OFD_STRING("ushort"),
            OFD_STRING("using"),
            OFD_STRING("virtual"),
            OFD_STRING("void"),
            OFD_STRING("volatile"),
            OFD_STRING("while"),
        };
        
        ofd_apply_c_syntax_highlighting(result_html, text, Ofd_String_("//"), Ofd_String_("/*"), Ofd_String_("*/"), cs_keywords, ofd_array_count(cs_keywords), 0);
    }
    else if(ofd_string_matches(language, "java"))
    {
        const Ofd_String java_keywords[] = { // Taken from https://docs.oracle.com/javase/tutorial/java/nutsandbolts/_keywords.html
            OFD_STRING("abstract"),
            OFD_STRING("assert"),
            OFD_STRING("boolean"),
            OFD_STRING("break"),
            OFD_STRING("byte"),
            OFD_STRING("case"),
            OFD_STRING("catch"),
            OFD_STRING("char"),
            OFD_STRING("class"),
            OFD_STRING("const"),
            OFD_STRING("continue"),
            OFD_STRING("default"),
            OFD_STRING("do"),
            OFD_STRING("double"),
            OFD_STRING("else"),
            OFD_STRING("enum"),
            OFD_STRING("extends"),
            OFD_STRING("final"),
            OFD_STRING("finally"),
            OFD_STRING("float"),
            OFD_STRING("for"),
            OFD_STRING("goto"),
            OFD_STRING("if"),
            OFD_STRING("implements"),
            OFD_STRING("import"),
            OFD_STRING("instanceof"),
            OFD_STRING("int"),
            OFD_STRING("interface"),
            OFD_STRING("long"),
            OFD_STRING("native"),
            OFD_STRING("new"),
            OFD_STRING("package"),
            OFD_STRING("private"),
            OFD_STRING("protected"),
            OFD_STRING("public"),
            OFD_STRING("return"),
            OFD_STRING("short"),
            OFD_STRING("static"),
            OFD_STRING("strictfp"),
            OFD_STRING("super"),
            OFD_STRING("switch"),
            OFD_STRING("synchronized"),
            OFD_STRING("this"),
            OFD_STRING("throw"),
            OFD_STRING("throws"),
            OFD_STRING("transient"),
            OFD_STRING("try"),
            OFD_STRING("void"),
            OFD_STRING("volatile"),
            OFD_STRING("while"),
        };
        
        ofd_apply_c_syntax_highlighting(result_html, text, Ofd_String_("//"), Ofd_String_("/*"), Ofd_String_("*/"), java_keywords, ofd_array_count(java_keywords), 0);
    }
    else if(ofd_string_matches(language, "perl"))
    {
        const Ofd_String perl_keywords[] = { // Taken from https://learn.perl.org/docs/keywords.html
            OFD_STRING("__DATA__"),
            OFD_STRING("__END__"),
            OFD_STRING("__FILE__"),
            OFD_STRING("__LINE__"),
            OFD_STRING("__PACKAGE__"),
            OFD_STRING("and"),
            OFD_STRING("cmp"),
            OFD_STRING("continue"),
            OFD_STRING("CORE"),
            OFD_STRING("do"),
            OFD_STRING("else"),
            OFD_STRING("elsif"),
            OFD_STRING("eq"),
            OFD_STRING("exp"),
            OFD_STRING("for"),
            OFD_STRING("foreach"),
            OFD_STRING("ge"),
            OFD_STRING("gt"),
            OFD_STRING("if"),
            OFD_STRING("le"),
            OFD_STRING("lock"),
            OFD_STRING("lt"),
            OFD_STRING("m"),
            OFD_STRING("ne"),
            OFD_STRING("no"),
            OFD_STRING("or"),
            OFD_STRING("package"),
            OFD_STRING("q"),
            OFD_STRING("qq"),
            OFD_STRING("qr"),
            OFD_STRING("qw"),
            OFD_STRING("qx"),
            OFD_STRING("s"),
            OFD_STRING("sub"),
            OFD_STRING("tr"),
            OFD_STRING("unless"),
            OFD_STRING("until"),
            OFD_STRING("while"),
            OFD_STRING("xor"),
            OFD_STRING("y"),
        };
        
        ofd_apply_c_syntax_highlighting(result_html, text, Ofd_String_("#"), Ofd_String_(""), Ofd_String_(""), perl_keywords, ofd_array_count(perl_keywords), 0);
    }
    else if(ofd_string_matches(language, "py") || ofd_string_matches(language, "python"))
    {
        const Ofd_String py_keywords[] = {
            OFD_STRING("and"),
            OFD_STRING("as"),
            OFD_STRING("assert"),
            OFD_STRING("break"),
            OFD_STRING("class"),
            OFD_STRING("continue"),
            OFD_STRING("def"),
            OFD_STRING("del"),
            OFD_STRING("elif"),
            OFD_STRING("else"),
            OFD_STRING("except"),
            OFD_STRING("exec"),
            OFD_STRING("finally"),
            OFD_STRING("for"),
            OFD_STRING("from"),
            OFD_STRING("global"),
            OFD_STRING("if"),
            OFD_STRING("import"),
            OFD_STRING("in"),
            OFD_STRING("is"),
            OFD_STRING("lambda"),
            OFD_STRING("not"),
            OFD_STRING("or"),
            OFD_STRING("pass"),
            OFD_STRING("print"),
            OFD_STRING("raise"),
            OFD_STRING("return"),
            OFD_STRING("try"),
            OFD_STRING("while"),
            OFD_STRING("with"),
            OFD_STRING("yield"),
        };
        
        
        ofd_apply_c_syntax_highlighting(result_html, text, Ofd_String_("#"), Ofd_String_("\"\"\""), Ofd_String_("\"\"\""), py_keywords, ofd_array_count(py_keywords), 0);
    }
    else if(ofd_string_matches(language, "js") || ofd_string_matches(language, "javascript"))
    {
        const Ofd_String js_keywords[] = { // Taken from https://www.programiz.com/javascript/keywords-identifiers
            OFD_STRING("await"),
            OFD_STRING("break"),
            OFD_STRING("case"),
            OFD_STRING("catch"),
            OFD_STRING("class"),
            OFD_STRING("const"),
            OFD_STRING("continue"),
            OFD_STRING("debugger"),
            OFD_STRING("default"),
            OFD_STRING("delete"),
            OFD_STRING("do"),
            OFD_STRING("else"),
            OFD_STRING("enum"),
            OFD_STRING("export"),
            OFD_STRING("extends"),
            OFD_STRING("false"),
            OFD_STRING("finally"),
            OFD_STRING("for"),
            OFD_STRING("function"),
            OFD_STRING("if"),
            OFD_STRING("implements"),
            OFD_STRING("import"),
            OFD_STRING("in"),
            OFD_STRING("instanceof"),
            OFD_STRING("interface"),
            OFD_STRING("let"),
            OFD_STRING("new"),
            OFD_STRING("null"),
            OFD_STRING("package"),
            OFD_STRING("private"),
            OFD_STRING("protected"),
            OFD_STRING("public"),
            OFD_STRING("return"),
            OFD_STRING("super"),
            OFD_STRING("switch"),
            OFD_STRING("static"),
            OFD_STRING("this"),
            OFD_STRING("throw"),
            OFD_STRING("try"),
            OFD_STRING("true"),
            OFD_STRING("typeof"),
            OFD_STRING("var"),
            OFD_STRING("void"),
            OFD_STRING("while"),
            OFD_STRING("with"),
            OFD_STRING("yield"),
        };
        
        ofd_apply_c_syntax_highlighting(result_html, text, Ofd_String_("//"), Ofd_String_("/*"), Ofd_String_("*/"), js_keywords, ofd_array_count(js_keywords), 0);
    }
    else if(ofd_string_matches(language, "ts") || ofd_string_matches(language, "typescript"))
    {
        const Ofd_String ts_keywords[] = { // Taken from https://github.com/microsoft/TypeScript/issues/2536
            OFD_STRING("break"),
            OFD_STRING("case"),
            OFD_STRING("catch"),
            OFD_STRING("class"),
            OFD_STRING("const"),
            OFD_STRING("continue"),
            OFD_STRING("debugger"),
            OFD_STRING("default"),
            OFD_STRING("delete"),
            OFD_STRING("do"),
            OFD_STRING("else"),
            OFD_STRING("enum"),
            OFD_STRING("export"),
            OFD_STRING("extends"),
            OFD_STRING("false"),
            OFD_STRING("finally"),
            OFD_STRING("for"),
            OFD_STRING("function"),
            OFD_STRING("if"),
            OFD_STRING("import"),
            OFD_STRING("in"),
            OFD_STRING("instanceof"),
            OFD_STRING("new"),
            OFD_STRING("null"),
            OFD_STRING("return"),
            OFD_STRING("super"),
            OFD_STRING("switch"),
            OFD_STRING("this"),
            OFD_STRING("throw"),
            OFD_STRING("true"),
            OFD_STRING("try"),
            OFD_STRING("typeof"),
            OFD_STRING("var"),
            OFD_STRING("void"),
            OFD_STRING("while"),
            OFD_STRING("with"),
            OFD_STRING("as"),
            OFD_STRING("implements"),
            OFD_STRING("interface"),
            OFD_STRING("let"),
            OFD_STRING("package"),
            OFD_STRING("private"),
            OFD_STRING("protected"),
            OFD_STRING("public"),
            OFD_STRING("static"),
            OFD_STRING("yield"),
            OFD_STRING("any"),
            OFD_STRING("boolean"),
            OFD_STRING("constructor"),
            OFD_STRING("declare"),
            OFD_STRING("get"),
            OFD_STRING("module"),
            OFD_STRING("require"),
            OFD_STRING("number"),
            OFD_STRING("set"),
            OFD_STRING("string"),
            OFD_STRING("symbol"),
            OFD_STRING("type"),
            OFD_STRING("from"),
            OFD_STRING("of"),
        };
        
        ofd_apply_c_syntax_highlighting(result_html, text, Ofd_String_("//"), Ofd_String_("/*"), Ofd_String_("*/"), ts_keywords, ofd_array_count(ts_keywords), 0);
    }
    else if(ofd_string_matches(language, "php"))
    {
        const Ofd_String php_keywords[] = { // Taken from https://www.php.net/manual/en/reserved.keywords.php
            OFD_STRING("__halt_compiler"),
            OFD_STRING("abstract"),
            OFD_STRING("and"),
            OFD_STRING("array"),
            OFD_STRING("as"),
            OFD_STRING("break"),
            OFD_STRING("callable"),
            OFD_STRING("case"),
            OFD_STRING("catch"),
            OFD_STRING("class"),
            OFD_STRING("clone"),
            OFD_STRING("const"),
            OFD_STRING("continue"),
            OFD_STRING("declare"),
            OFD_STRING("default"),
            OFD_STRING("die"),
            OFD_STRING("do"),
            OFD_STRING("echo"),
            OFD_STRING("else"),
            OFD_STRING("elseif"),
            OFD_STRING("empty"),
            OFD_STRING("enddeclare"),
            OFD_STRING("endfor"),
            OFD_STRING("endforeach"),
            OFD_STRING("endif"),
            OFD_STRING("endswitch"),
            OFD_STRING("endwhile"),
            OFD_STRING("eval"),
            OFD_STRING("exit"),
            OFD_STRING("extends"),
            OFD_STRING("final"),
            OFD_STRING("finally"),
            OFD_STRING("fn"),
            OFD_STRING("for"),
            OFD_STRING("foreach"),
            OFD_STRING("function"),
            OFD_STRING("global"),
            OFD_STRING("goto"),
            OFD_STRING("if"),
            OFD_STRING("implements"),
            OFD_STRING("include"),
            OFD_STRING("include_once"),
            OFD_STRING("instanceof"),
            OFD_STRING("insteadof"),
            OFD_STRING("interface"),
            OFD_STRING("isset"),
            OFD_STRING("list"),
            OFD_STRING("match"),
            OFD_STRING("namespace"),
            OFD_STRING("new"),
            OFD_STRING("or"),
            OFD_STRING("print"),
            OFD_STRING("private"),
            OFD_STRING("protected"),
            OFD_STRING("public"),
            OFD_STRING("readonly"),
            OFD_STRING("require"),
            OFD_STRING("require_once"),
            OFD_STRING("return"),
            OFD_STRING("static"),
            OFD_STRING("switch"),
            OFD_STRING("throw"),
            OFD_STRING("trait"),
            OFD_STRING("try"),
            OFD_STRING("unset"),
            OFD_STRING("use"),
            OFD_STRING("var"),
            OFD_STRING("while"),
            OFD_STRING("xor"),
            OFD_STRING("yield"),
        };
        
        ofd_apply_c_syntax_highlighting(result_html, text, Ofd_String_("//"), Ofd_String_("/*"), Ofd_String_("*/"), php_keywords, ofd_array_count(php_keywords), 0);
    }
    else if(ofd_string_matches(language, "bat"))
    {
        const Ofd_String bat_keywords[] = { // Taken from https://en.m.wikibooks.org/wiki/Windows_Batch_Scripting#Built-in_commands
            OFD_STRING("ASSOC"),
            OFD_STRING("BREAK"),
            OFD_STRING("CALL"),
            OFD_STRING("CD"),
            OFD_STRING("CHDIR"),
            OFD_STRING("CHCP"),
            OFD_STRING("CLS"),
            OFD_STRING("COLOR"),
            OFD_STRING("COPY"),
            OFD_STRING("DATE"),
            OFD_STRING("DEL"),
            OFD_STRING("ERASE"),
            OFD_STRING("DIR"),
            OFD_STRING("ECHO"),
            OFD_STRING("ELSE"),
            OFD_STRING("ENDLOCAL"),
            OFD_STRING("EXIT"),
            OFD_STRING("FOR"),
            OFD_STRING("FTYPE"),
            OFD_STRING("GOTO"),
            OFD_STRING("IF"),
            OFD_STRING("MD"),
            OFD_STRING("MKDIR"),
            OFD_STRING("MOVE"),
            OFD_STRING("PATH"),
            OFD_STRING("PAUSE"),
            OFD_STRING("POPD"),
            OFD_STRING("PROMPT"),
            OFD_STRING("PUSHD"),
            OFD_STRING("RD"),
            OFD_STRING("RMDIR"),
            OFD_STRING("REN"),
            OFD_STRING("RENAME"),
            OFD_STRING("SET"),
            OFD_STRING("SETLOCAL"),
            OFD_STRING("SHIFT"),
            OFD_STRING("START"),
            OFD_STRING("TIME"),
            OFD_STRING("TITLE"),
            OFD_STRING("TYPE"),
            OFD_STRING("VER"),
            OFD_STRING("VERIFY"),
            OFD_STRING("VOL"),
            
            // Same keywords but in lower case.
            OFD_STRING("assoc"),
            OFD_STRING("break"),
            OFD_STRING("call"),
            OFD_STRING("cd"),
            OFD_STRING("chdir"),
            OFD_STRING("chcp"),
            OFD_STRING("cls"),
            OFD_STRING("color"),
            OFD_STRING("copy"),
            OFD_STRING("date"),
            OFD_STRING("del"),
            OFD_STRING("erase"),
            OFD_STRING("dir"),
            OFD_STRING("echo"),
            OFD_STRING("else"),
            OFD_STRING("endlocal"),
            OFD_STRING("exit"),
            OFD_STRING("for"),
            OFD_STRING("ftype"),
            OFD_STRING("goto"),
            OFD_STRING("if"),
            OFD_STRING("md"),
            OFD_STRING("mkdir"),
            OFD_STRING("move"),
            OFD_STRING("path"),
            OFD_STRING("pause"),
            OFD_STRING("popd"),
            OFD_STRING("prompt"),
            OFD_STRING("pushd"),
            OFD_STRING("rd"),
            OFD_STRING("rmdir"),
            OFD_STRING("ren"),
            OFD_STRING("rename"),
            OFD_STRING("set"),
            OFD_STRING("setlocal"),
            OFD_STRING("shift"),
            OFD_STRING("start"),
            OFD_STRING("time"),
            OFD_STRING("title"),
            OFD_STRING("type"),
            OFD_STRING("ver"),
            OFD_STRING("verify"),
            OFD_STRING("vol"),
        };
        
        ofd_apply_c_syntax_highlighting(result_html, text, Ofd_String_("REM"), Ofd_String_(""), Ofd_String_(""), bat_keywords, ofd_array_count(bat_keywords), 0);
    }
    else if(ofd_string_matches(language, "sh") || ofd_string_matches(language, "bash"))
    {
        const Ofd_String sh_keywords[] = { // Taken from https://www.gnu.org/software/bash/manual/html_node/Reserved-Words.html
            OFD_STRING("case"),
            OFD_STRING("coproc"),
            OFD_STRING("do"),
            OFD_STRING("done"),
            OFD_STRING("elif"),
            OFD_STRING("else"),
            OFD_STRING("esac"),
            OFD_STRING("fi"),
            OFD_STRING("for"),
            OFD_STRING("function"),
            OFD_STRING("if"),
            OFD_STRING("in"),
            OFD_STRING("select"),
            OFD_STRING("then"),
            OFD_STRING("time"),
            OFD_STRING("until"),
            OFD_STRING("while"),
        };
        
        ofd_apply_c_syntax_highlighting(result_html, text, Ofd_String_("#"), Ofd_String_(""), Ofd_String_(""), sh_keywords, ofd_array_count(sh_keywords), 0);
    }
    else if(ofd_string_matches(language, "json"))
    {
        const Ofd_String json_keywords[] = {
            OFD_STRING("false"),
            OFD_STRING("true"),
        };
        
        ofd_apply_c_syntax_highlighting(result_html, text, Ofd_String_("//"), Ofd_String_("/*"), Ofd_String_("*/"), json_keywords, ofd_array_count(json_keywords), 0);
    }
    else if(ofd_string_matches(language, "css"))
    {
        ofd_apply_c_syntax_highlighting(result_html, text, Ofd_String_(""), Ofd_String_("/*"), Ofd_String_("*/"), NULL, 0, 0);
    }
    else if(ofd_string_matches(language, "xml") || ofd_string_matches(language, "html"))
    {
        // Parse XML. START
        char* c     = text.data;
        char* limit = text.data + text.count;
        
        ofd_b8 we_are_inside_a_tag = ofd_false;
        
        Ofd_String basic_text = {c};
        
        while(c < limit)
        {
            char character = *c;
            switch(character)
            {
                case '<':
                {
                    if(we_are_inside_a_tag)
                    {
                        ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-syntax ofd-syntax-keyword'>"));
                        ofd_syntax_spill_basic_text_no_keywords(result_html, &basic_text);
                        ofd_array_add_string(result_html, Ofd_String_("</div>"));
                    }
                    else
                    {
                        ofd_syntax_spill_basic_text_no_keywords(result_html, &basic_text);
                    }
                    
                    
                    const char* comment_start = "<!--";
                    const char* comment_end   = "-->";
                    int comment_start_count = ofd_strlen(comment_start);
                    int comment_end_count   = ofd_strlen(comment_end);
                    if(c + comment_start_count <= limit && ofd_memcmp(c, comment_start, comment_start_count) == 0)
                    {
                        // Handle a comment. START
                        Ofd_String token = {c};
                        c += comment_start_count;
                        while(c < limit)
                        {
                            if(c + comment_end_count <= limit && ofd_memcmp(c, comment_end, comment_end_count) == 0)
                            {
                                c += comment_end_count;
                                break;
                            }
                            
                            c++;
                        }
                        
                        token.count = c - token.data;
                        
                        ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-syntax ofd-syntax-comment'>"));
                        ofd_syntax_insert_token(result_html, token);
                        ofd_array_add_string(result_html, Ofd_String_("</div>"));
                        // Handle a comment. END
                        
                        break;
                    }
                    
                    
                    we_are_inside_a_tag = ofd_true;
                    
                    ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-syntax ofd-syntax-structure'>&lt"));
                    
                    if(c + 1 < limit && c[1] == '/')
                    {
                        c++;
                        ofd_array_add_string(result_html, Ofd_String_("/"));
                    }
                    
                    ofd_array_add_string(result_html, Ofd_String_("</div>"));
                    
                    c++;
                } break;
                
                case '>':
                {
                    if(we_are_inside_a_tag)
                    {
                        ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-syntax ofd-syntax-keyword'>"));
                        ofd_syntax_spill_basic_text_no_keywords(result_html, &basic_text);
                        ofd_array_add_string(result_html, Ofd_String_("</div>"));
                    }
                    else
                    {
                        ofd_syntax_spill_basic_text_no_keywords(result_html, &basic_text);
                    }
                    
                    we_are_inside_a_tag = ofd_false;
                    
                    ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-syntax ofd-syntax-structure'>&gt</div>"));
                    c++;
                } break;
                
                
                case '"':
                case '\'':
                {
                    if(we_are_inside_a_tag)
                    {
                        ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-syntax ofd-syntax-keyword'>"));
                        ofd_syntax_spill_basic_text_no_keywords(result_html, &basic_text);
                        ofd_array_add_string(result_html, Ofd_String_("</div>"));
                    }
                    else
                    {
                        ofd_syntax_spill_basic_text_no_keywords(result_html, &basic_text);
                    }
                    
                    
                    c = ofd_syntax_handle_string(result_html, c, limit);
                } break;
                
                
                default:
                {
                    if(!basic_text.count) basic_text.data = c;
                    basic_text.count++;
                    c++;
                } break;
            }
        }
        
        
        if(we_are_inside_a_tag)
        {
            ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-syntax ofd-syntax-keyword'>"));
            ofd_syntax_spill_basic_text_no_keywords(result_html, &basic_text);
            ofd_array_add_string(result_html, Ofd_String_("</div>"));
        }
        else
        {
            ofd_syntax_spill_basic_text_no_keywords(result_html, &basic_text);
        }
        // Parse XML. END
    }
    else
    {
        ofd_b8 was_filtered = ofd_filter_plain_text(&text);
        ofd_array_add_string(result_html, text);
        if(was_filtered) ofd_free(text.data);
    }
}



ofd_static void ofd_parse_markdown(char* c, char* limit, Ofd_Array* result_html, Ofd_Array* result_sections, Ofd_Array* link_references, int* next_section_id, Ofd_Array* log_data)
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE: result_html and result_sections must be initialised beforehand using ofd_array_init().
    ///////////////////////////////////////////////////////////////////////////////////////////////
    
    char* lower_limit = c;
    
    Ofd_String basic_text = {c};
    
    #define OFD_SPILL_TEXT()\
    {\
        ofd_b8 was_filtered = ofd_filter_plain_text(&basic_text);\
        ofd_array_add_string(result_html, basic_text);\
        if(was_filtered) ofd_free(basic_text.data);\
        \
        basic_text.count = 0;\
    }
    
    while(c < limit)
    {
        switch(*c)
        {
            case '#':
            {
                char* whitespace_start = c;
                while(whitespace_start > lower_limit)
                {
                    if(*whitespace_start == '\n' || *whitespace_start == '\r')
                    {
                        whitespace_start++;
                        break;
                    }
                    
                    whitespace_start--;
                }
                
                if(whitespace_start != c)
                {
                    char right_before = *(whitespace_start - 1);
                    if(right_before != '\n' && right_before != '\r') goto do_the_default_thing; // This does not appear to be an actual header.
                }
                
                
                // Handle a header. START
                OFD_SPILL_TEXT();
                
                int header_hierarchy = 0;
                
                while(c < limit)
                { // Get the header hierarchy.
                    if(*c != '#') break;
                    
                    header_hierarchy++;
                    c++;
                }
                
                header_hierarchy = ofd_min(header_hierarchy, OFD_MAX_HEADER_LEVEL);
                
                c = ofd_skip_whitespace(c, limit); // Skip whitespace to get to the header text.
                
                Ofd_String text = {c};
                while(c < limit)
                { // Get the header text.
                    if(*c == '\n' || *c == '\r') break;
                    
                    text.count++;
                    c++;
                }
                
                if(text.count)
                {
                    text.count = ofd_min(text.count, OFD_MAX_SECTION_NAME_COUNT);
                    
                    Ofd_Section* section = ofd_array_add_fast(result_sections);
                    section->id         = *next_section_id;
                    section->hierarchy  = header_hierarchy;
                    
                    Ofd_String section_name = ofd_cleanup_section_name(text, section->name_memory);
                    section->name_count = section_name.count;
                    
                    ofd_b8 this_is_not_the_root_header = header_hierarchy > 1 && result_sections->count > 1;
                    
                    Ofd_String section_id = ofd_make_section_id(section_name, section->id_memory);
                    section->id_count = section_id.count;
                    
                    if(this_is_not_the_root_header)
                    {
                        // Get the complete section ID. START
                        char final_result[OFD_MAX_SECTION_NAME_COUNT];
                        int final_count = 0;
                        
                        int higher_hierarchy = header_hierarchy - 1;
                        
                        char* current_destination = final_result + OFD_MAX_SECTION_NAME_COUNT - section_id.count;
                        ofd_copy(current_destination, section_id.data, section_id.count);
                        final_count += section_id.count;
                        
                        Ofd_Section* current_section     = section - 1;
                        Ofd_Section* section_lower_limit = ofd_cast(result_sections->data, Ofd_Section*);
                        while(current_section >= section_lower_limit)
                        {
                            if(current_section->hierarchy == 0) break;
                            
                            if(current_section->hierarchy <= higher_hierarchy)
                            {
                                current_destination--;
                                *current_destination = '-';
                                final_count++;
                                
                                current_destination -= current_section->id_count;
                                ofd_copy(current_destination, current_section->id_memory, current_section->id_count);
                                final_count += current_section->id_count;
                                
                                higher_hierarchy = current_section->hierarchy - 1;
                                if(higher_hierarchy <= 0) break;
                            }
                            
                            current_section--;
                        }
                        
                        ofd_copy(section->full_id_memory, current_destination, final_count);
                        
                        section->full_id_count = final_count;
                        section_id.count       = final_count;
                        section_id.data        = section->full_id_memory;
                        // Get the complete section ID. END
                    }
                    else
                    {
                        ofd_copy(section->full_id_memory, section->id_memory, section->id_count);
                        section->full_id_count = section->id_count;
                    }
                    
                    (*next_section_id)++;
                    
                    // Output HTML. START
                    char buffer[128];
                    //ofd_sprintf(buffer, "<div class='ofd-header ofd-header-%d' id='ofd-section-%d'>", section->hierarchy, section->id);
                    ofd_sprintf(buffer, "<div class='ofd-header ofd-header-%d' id='", section->hierarchy);
                    ofd_array_add_string(result_html, Ofd_String_(buffer));
                    ofd_array_add_string(result_html, section_id);
                    ofd_array_add_string(result_html, Ofd_String_("'>"));
                    
                    ofd_parse_markdown(text.data, text.data + text.count, result_html, result_sections, link_references, next_section_id, log_data);
                    ofd_array_add_string(result_html, Ofd_String_("</div>"));
                    // Output HTML. END
                }
                
                while(c < limit)
                { // Go to the next line.
                    if(*c != '\n' && *c != '\r') break;
                    c++;
                }
                // Handle a header. END
            } break;
            
            
            case '`':
            {
                // Handle a code block. START
                OFD_SPILL_TEXT();
                
                int backtick_count = 0;
                char* tmp_c = c;
                while(tmp_c < limit)
                {
                    if(*tmp_c != '`') break;
                    tmp_c++;
                    backtick_count++;
                }
                
                if(backtick_count > OFD_MAX_CODE_BLOCK_INDICATOR_COUNT) goto do_the_default_thing; // This does not appear to be a code block.
                
                c = tmp_c;
                
                ofd_b8 inline_the_block = backtick_count < OFD_MAX_CODE_BLOCK_INDICATOR_COUNT;
                
                Ofd_String language = {c};
                
                if(backtick_count < OFD_MAX_CODE_BLOCK_INDICATOR_COUNT)
                {
                    if(c < limit && *c == '\n')
                    {
                        inline_the_block = ofd_false;
                        
                        c++;
                        if(c < limit && *c == '\r') c++;
                    }
                }
                else
                { // This is a block where a programming language may be indicated.
                    while(c < limit)
                    { // Retrieve the indicated language.
                        char character = *c;
                        if(character == '\n' || character == '\r' || character == '`') break;
                        
                        language.count++;
                        c++;
                    }
                    
                    if(c < limit && *c == '\n') // Skip the new line.
                    {
                        c++;
                        if(c < limit && *c == '\r') c++;
                    }
                }
                
                
                const char block_end_indicator[] = {'`', '`', '`'};
                
                Ofd_String text = {c};
                
                while(c < limit)
                {
                    if(*c == '`' && c + backtick_count <= limit)
                    { // Check for a potential end of block.
                        //ofd_b8 this_is_the_end = (ofd_memcmp(c, block_end_indicator, backtick_count) == 0);
                        tmp_c = c;
                        while(tmp_c < limit)
                        {
                            if(*tmp_c != '`') break;
                            tmp_c++;
                        }
                        
                        int num_backticks_found = tmp_c - c;
                        ofd_b8 this_is_the_end = (num_backticks_found == backtick_count);
                        
                        char previous_character = *(c - 1);
                        
                        if(this_is_the_end)
                        {
                            c += backtick_count;
                            
                            if(previous_character == '\n' || previous_character == '\r')
                            {
                                if(c < limit && *c == '\n')
                                {
                                    c++;
                                    if(c < limit && *c == '\r') c++;
                                }
                            }
                            
                            break;
                        }
                        
                        c += num_backticks_found;
                        text.count += num_backticks_found;
                        continue;
                    }
                    
                    text.count++;
                    c++;
                }
                
                
                if(text.count)
                {
                    // Output HTML. START
                    char buffer[128];
                    ofd_sprintf(buffer, "<div class='ofd-code-block ofd-code-block-%u'>", inline_the_block);
                    
                    ofd_array_add_string(result_html, Ofd_String_(buffer));
                    
                    if(!language.count)
                    { // This is a simple code block.
                        ofd_b8 was_filtered = ofd_filter_plain_text(&text);
                        ofd_array_add_string(result_html, text);
                        if(was_filtered) ofd_free(text.data);
                    }
                    else
                    { // This is a code block where the language is indicated.
                        ofd_apply_syntax_highlighting(result_html, text, language);
                    }
                    
                    ofd_array_add_string(result_html, Ofd_String_("</div>"));
                    // Output HTML. END
                }
                // Handle a code block. END
            } break;
            
            
            case '~':
            {
                if(c + 1 < limit && c[1] == '~')
                {
                    // Handle strike-through. START
                    c += 2;
                    
                    const char pattern[2] = {'~', '~'};
                    
                    Ofd_String text = {c};
                    
                    while(c < limit)
                    {
                        if(ofd_memcmp(c, pattern, 2) == 0)
                        {
                            c += 2;
                            break;
                        }
                        
                        text.count++;
                        c++;
                    }
                    
                    
                    // Output HTML. START
                    ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-strike-through'>"));
                    ofd_parse_markdown(text.data, text.data + text.count, result_html, result_sections, link_references, next_section_id, log_data);
                    ofd_array_add_string(result_html, Ofd_String_("</div>"));
                    // Output HTML. END
                    // Handle strike-through. END
                    
                    break;
                }
                
                goto do_the_default_thing;
            } break;
            
            
            case '<':
            {
                if(c + 3 <= limit && ofd_memcmp(c, "</>", 3) == 0)
                {
                    // Handle embedded HTML. START
                    OFD_SPILL_TEXT();
                    
                    c += 3;
                    if(c < limit)
                    { // Skip new line.
                        if(*c == '\n')
                        {
                            c++;
                            if(c < limit && *c == '\r') c++;
                        }
                        else if(*c == '\r') c++;
                    }
                    
                    char* html_end = c;
                    while(html_end + 3 <= limit)
                    {
                        if(ofd_memcmp(html_end, "</>", 3) == 0) break;
                        html_end++;
                    }
                    
                    char* actual_html_end = html_end;
                    actual_html_end--;
                    if(actual_html_end >= c)
                    { // Remove extraneaous new line.
                        if(*actual_html_end == '\r')
                        {
                            actual_html_end--;
                            if(actual_html_end >= c && *actual_html_end == '\n') actual_html_end--;
                        }
                        else if(*actual_html_end == '\n') actual_html_end--;
                    }
                    actual_html_end++;
                    
                    Ofd_String html_to_embed = {c, actual_html_end - c};
                    ofd_array_add_string(result_html, html_to_embed);
                    
                    if(html_end < limit)
                    {
                        c = html_end + 3;
                        if(c < limit)
                        { // Skip new line.
                            if(*c == '\n')
                            {
                                c++;
                                if(c < limit && *c == '\r') c++;
                            }
                            else if(*c == '\r') c++;
                        }
                    }
                    else c = html_end;
                    // Handle embedded HTML. END
                    
                    break;
                }
                
                goto do_the_default_thing;
            } break;
            
            
            case '>':
            {
                ofd_b8 this_is_a_quote = ofd_true;
                
                char* tmp_c = c;
                tmp_c--;
                while(tmp_c >= lower_limit)
                { // If characters up to the previous line are not whitespace then we do not consider this a quote.
                    char character = *tmp_c;
                    if(character == '\n' || character == '\r') break;
                    if(character != ' ' && character != '\t')
                    {
                        this_is_a_quote = ofd_false;
                        break;
                    }
                    
                    tmp_c--;
                }
                
                if(this_is_a_quote)
                {
                    // Handle a quote. START
                    OFD_SPILL_TEXT();
                    
                    tmp_c++;
                    Ofd_String whitespace = {tmp_c, c - tmp_c};
                    
                    ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-quote'>"));
                    
                    tmp_c = c;
                    while(tmp_c < limit)
                    {
                        char* quote_start = tmp_c;
                        
                        // Retrieve line end.
                        char* line_end = quote_start;
                        while(line_end < limit)
                        {
                            if(*line_end == '\n' || *line_end == '\r') break;
                            line_end++;
                        }
                        /////////////////////
                        
                        tmp_c = quote_start + 1; // Skip '>'.
                        tmp_c = ofd_skip_whitespace(tmp_c, limit);
                        
                        // Parse quote data.
                        ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-quote-line'>"));
                        ofd_parse_markdown(tmp_c, line_end, result_html, result_sections, link_references, next_section_id, log_data);
                        ofd_array_add_string(result_html, Ofd_String_("</div>"));
                        ////////////////////
                        
                        // Check wether the quote continues on the next line. START
                        if(line_end >= limit)
                        {
                            tmp_c = line_end;
                            break;
                        }
                        
                        tmp_c = line_end + 1;
                        if(tmp_c >= limit) break;
                        
                        if(*line_end == '\n' && *tmp_c == '\r') tmp_c++;
                        
                        if(tmp_c + whitespace.count + 1 <= limit) // '+ 1' to check for the '>' that continues the quote.
                        {
                            if(ofd_memcmp(tmp_c, whitespace.data, whitespace.count) == 0)
                            {
                                if(tmp_c[whitespace.count] == '>') tmp_c += whitespace.count;
                                else break; // It looks like the quote does not continue there.
                            }
                            else break; // The whitespace on the next line does not match the one on this line so the quote does not continue there.
                        }
                        else break;
                        // Check wether the quote continues on the next line. END
                    }
                    
                    ofd_array_add_string(result_html, Ofd_String_("</div>"));
                    // Handle a quote. END
                    
                    c = tmp_c;
                    break;
                }
                
                goto do_the_default_thing;
            } break;
            
            
            case '*':
            {
                if(c + 1 < limit && (c[1] == ' ' || c[1] == '\t')) goto handle_a_list;
            }
            case '_':
            {
                char* tmp_c = c;
                
                char indicator = *tmp_c;
                char* next_c = ofd_maybe_handle_a_horizontal_rule(c, limit, indicator, result_html, &basic_text);
                if(next_c)
                {
                    c = next_c;
                    break; // This is a horizontal rule.
                }
                
                
                char full_indicator[2] = {indicator, indicator};
                
                int emphasis_level = 0;
                while(tmp_c < limit)
                {
                    if(*tmp_c != indicator) break;
                    
                    emphasis_level++;
                    tmp_c++;
                }
                
                if(emphasis_level <= 2 && tmp_c < limit && *tmp_c != ' ' && *tmp_c != '\t')
                {
                    // Handle emphasis. START
                    OFD_SPILL_TEXT();
                    
                    c = tmp_c;
                    
                    Ofd_String text = {c};
                    while(c < limit)
                    {
                        char previous_character = *(c - 1);
                        if(previous_character != ' ' && previous_character != '\t')
                        {
                            if(ofd_memcmp(c, full_indicator, emphasis_level) == 0)
                            {
                                c += emphasis_level;
                                break;
                            }
                        }
                        
                        text.count++;
                        c++;
                    }
                    
                    // Output HTML. START
                    char buffer[64];
                    ofd_sprintf(buffer, "<div class='ofd-emphasis-%d'>", emphasis_level);
                    
                    ofd_array_add_string(result_html, Ofd_String_(buffer));
                    ofd_parse_markdown(text.data, text.data + text.count, result_html, result_sections, link_references, next_section_id, log_data);
                    ofd_array_add_string(result_html, Ofd_String_("</div>"));
                    // Output HTML. END
                    // Handle emphasis. END
                    
                    break;
                }
                
                goto do_the_default_thing;
            } break;
            
            
            case '-':
            {
                char* tmp_c = ofd_maybe_handle_a_horizontal_rule(c, limit, '-', result_html, &basic_text);
                if(tmp_c)
                { // This is a horizontal rule.
                    c = tmp_c;
                    break;
                }
            }
            case '+':
            {
                if(c + 1 < limit && (c[1] == ' ' || c[1] == '\t'))
                {
                    handle_a_list:;
                    // Check if this is in the middle of something else. START
                    char* previous_c = c - 1;
                    while(previous_c >= lower_limit)
                    {
                        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                        // NOTE: we check wether all characters up to the previous line end are whitespace. If that is not the case, we assume this is not
                        // supposed to be a list item.
                        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                        
                        char character = *previous_c;
                        if(character == ' ' || character == '\t')
                        { // Skip whitespace.
                            previous_c--;
                            continue;
                        }
                        
                        if(character == '\n' || character == '\r') break; // We only encountered whitespace so we assume this is a list item.
                        
                        goto do_the_default_thing; // This does not appear to be a list item.
                    }
                    // Check if this is in the middle of something else. END
                    
                    
                    OFD_SPILL_TEXT();
                    
                    ////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // NOTE: for now we only accept multi-line list items lines are aligned in the following way:
                    // - List item line 1.
                    //   List item line 2.
                    //  List item line 3 but actually this line is not part of the list item as it is not correctly aligned.
                    ////////////////////////////////////////////////////////////////////////////////////////////////////////
                    
                    // Get the end of this list item. START
                    ofd_b8 changed_line = ofd_false;
                    
                    char* tmp_c = c;
                    
                    tmp_c += 2;
                    while(tmp_c < limit)
                    {
                        switch(*tmp_c)
                        {
                            case '\n':
                            {
                                changed_line = ofd_true;
                                tmp_c++;
                                if(tmp_c < limit && *tmp_c == '\r') c++;
                                
                                int whitespace_count = 0;
                                while(tmp_c < limit)
                                {
                                    if(*tmp_c != ' ' && *tmp_c != '\t') break;
                                    whitespace_count++;
                                    tmp_c++;
                                }
                                
                                if(whitespace_count != 2)
                                {
                                    tmp_c -= whitespace_count;
                                    goto found_end_of_item;
                                }
                                
                                changed_line = ofd_false;
                            } break;
                            
                            case '*':
                            case '-':
                            case '+':
                            {
                                if(changed_line && tmp_c + 1 < limit && (tmp_c[1] == ' ' || tmp_c[1] == '\t')) goto found_end_of_item;
                                
                                tmp_c++;
                            } break;
                            
                            default: tmp_c++;
                        }
                    }
                    
                    found_end_of_item:;
                    char* end_of_item = tmp_c;
                    
                    ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-list-item'>"));
                    ofd_parse_markdown(c + 2, end_of_item, result_html, result_sections, link_references, next_section_id, log_data);
                    ofd_array_add_string(result_html, Ofd_String_("</div>"));
                    
                    c = end_of_item;
                    // Get the end of this list item. END
                    
                    break;
                }
                
                goto do_the_default_thing;
            } break;
            
            
            case '[':
            {
                // Handle a link. START
                Ofd_String link_name, link_address, link_title;
                Ofd_Array additional_css;
                
                char* original_c = c;
                
                // Check if this is a link reference. START
                char* tmp_c = c;
                tmp_c++;
                link_name.data = tmp_c;
                
                while(tmp_c < limit)
                {
                    char character = *tmp_c;
                    if(character == ']') break;
                    tmp_c++;
                }
                
                link_name.count = tmp_c - link_name.data;
                if(tmp_c + 1 < limit && tmp_c[1] == ':')
                { // We assume this is a reference.
                    // Parse a link reference. START
                    OFD_SPILL_TEXT();
                    
                    link_address.count = 0;
                    link_title.count   = 0;
                    ofd_array_init(&additional_css, 1);
                    
                    c = tmp_c;
                    c += 2; // Skip ']:'.
                    c = ofd_skip_whitespace(c, limit);
                    
                    // Retrieve the address. START
                    link_address.data = c;
                    while(c < limit)
                    {
                        char character = *c;
                        if(character == ' ' || character == '\t' || character == '\n' || character == '\r') break;
                        c++;
                    }
                    
                    link_address.count = c - link_address.data;
                    // Retrieve the address. END
                    
                    c = ofd_skip_whitespace(c, limit);
                    
                    while(c < limit)
                    {
                        char character = *c;
                        if(character == '\n' || character == '\r') break;
                        
                        if(character == '"')
                        {
                            // Parse a link title. START
                            c++;
                            link_title.data = c;
                            while(c < limit)
                            {
                                character = *c;
                                if(character == '\n' || character == '\r' || character == '"') break;
                                c++;
                            }
                            
                            link_title.count = c - link_title.data;
                            if(c < limit && *c =='"') c++;
                            // Parse a link title. END
                            
                            continue;
                        }
                        
                        if(character == 'w' || character == 'h' || character == 'r')
                        {
                            // Parse additional CSS. START
                            char original_character = character;
                            
                            while(c < limit)
                            {
                                character = *c;
                                if(character == '"' || character == '\n' || character == '\r') break;
                                c++;
                            }
                            
                            if(c == limit) break;
                            
                            if(*c == '"')
                            {
                                c++;
                                Ofd_String css = {c};
                                while(c < limit)
                                {
                                    character = *c;
                                    if(character == '"' || character == '\n' || character == '\r') break;
                                    c++;
                                }
                                
                                if(c == limit) break;
                                
                                if(*c == '"')
                                {
                                    css.count = c - css.data;
                                    switch(original_character)
                                    {
                                        case 'w':
                                        {
                                            ofd_array_add_string(&additional_css, Ofd_String_("width:"));
                                        } break;
                                        
                                        case 'h':
                                        {
                                            ofd_array_add_string(&additional_css, Ofd_String_("height:"));
                                        } break;
                                        
                                        case 'r':
                                        {
                                            ofd_array_add_string(&additional_css, Ofd_String_("border-radius:"));
                                        } break;
                                    }
                                    
                                    ofd_array_add_string(&additional_css, css);
                                    ofd_array_add_string(&additional_css, Ofd_String_(";"));
                                    
                                    c++;
                                    continue;
                                }
                            }
                            // Parse additional CSS. END
                        }
                        
                        c++;
                    }
                    
                    
                    ofd_b8 this_reference_exists = ofd_false;
                    for(int i = 0; i < link_references->count; i++)
                    {
                        Ofd_Link_Reference* ref = ofd_cast(link_references->data, Ofd_Link_Reference*) + i;
                        if(ofd_strings_match(ref->id, link_name))
                        {
                            // This reference already exists. START
                            char c_ref_id[128];
                            ofd_to_c_string(link_name, c_ref_id);
                            
                            char tmp_buffer[512];
                            ofd_sprintf(tmp_buffer, "A link/image reference named '%s' already exists.", c_ref_id);
                            
                            Ofd_String line_view = ofd_get_line_view(original_c, lower_limit, limit);
                            ofd_log_error(log_data, line_view, Ofd_String_(tmp_buffer));
                            
                            this_reference_exists = ofd_true;
                            // This reference already exists. END
                            
                            break;
                        }
                    }
                    
                    if(!this_reference_exists)
                    {
                        Ofd_Link_Reference* reference = ofd_array_add_fast(link_references);
                        reference->id             = link_name;
                        reference->link_address   = link_address;
                        reference->link_title     = link_title;
                        reference->additional_css = additional_css;
                    }
                    
                    c = ofd_skip_whitespace(c, limit);
                    if(c < limit)
                    { // Skip new line.
                        if(*c == '\n')
                        {
                            c++;
                            if(c < limit && *c == '\r') c++;
                        }
                        else if(*c == '\r') c++;
                    }
                    // Parse a link reference. END
                    
                    break;
                }
                // Check if this is a link reference. END
                
                
                tmp_c = c;
                tmp_c = ofd_parse_link(tmp_c, limit, link_references, &link_name, &link_address, &link_title, &additional_css, log_data);
                if(tmp_c)
                { // This is definitely a link.
                    OFD_SPILL_TEXT();
                    
                    // Output HTML. START
                    ofd_b8 this_is_an_external_link = ofd_true;
                    if(!link_address.count || link_address.data[0] == '#' | link_address.data[0] == '/') this_is_an_external_link = ofd_false;
                    
                    if(this_is_an_external_link)
                    {
                        ofd_array_add_string(result_html, Ofd_String_("<a class='ofd-external-link' target='_blank' href='"));
                    }
                    else
                    {
                        ofd_array_add_string(result_html, Ofd_String_("<a class='ofd-external-link' href='"));
                    }
                    
                    ofd_array_add_string(result_html, link_address);
                    
                    if(link_title.count)
                    {
                        ofd_array_add_string(result_html, Ofd_String_("' title='"));
                        ofd_array_add_string(result_html, link_title);
                    }
                    
                    ofd_array_add_string(result_html, Ofd_String_("'>"));
                    
                    ofd_parse_markdown(link_name.data, link_name.data + link_name.count, result_html, result_sections, link_references, next_section_id, log_data);
                    
                    ofd_array_add_string(result_html, Ofd_String_("</a>"));
                    
                    ofd_free(additional_css.data); // Just in case.
                    // Output HTML. END
                    
                    c = tmp_c;
                    break;
                }
                
                goto do_the_default_thing;
                // Handle a link. END
            } break;
            
            
            case '!':
            {
                char* tmp_c = c;
                tmp_c++;
                if(tmp_c < limit && *tmp_c == '[')
                {
                    // Handle an image. START
                    OFD_SPILL_TEXT();
                    
                    Ofd_String image_name, image_address, image_title;
                    Ofd_Array additional_css;
                    
                    tmp_c = ofd_parse_link(tmp_c, limit, link_references, &image_name, &image_address, &image_title, &additional_css, log_data);
                    if(tmp_c)
                    { // This is definitely an image.
                        // Output HTML. START
                        ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-image-container'><img class='ofd-basic-image' src='"));
                        ofd_array_add_string(result_html, image_address);
                        ofd_array_add_string(result_html, Ofd_String_("' alt='"));
                        ofd_array_add_string(result_html, image_name);
                        
                        if(additional_css.count)
                        {
                            ofd_array_add_string(result_html, Ofd_String_("' style='"));
                            
                            Ofd_String css = {additional_css.data, additional_css.count};
                            ofd_array_add_string(result_html, css);
                            
                            ofd_free(additional_css.data);
                        }
                        
                        ofd_array_add_string(result_html, Ofd_String_("' />"));
                        
                        if(image_title.count)
                        {
                            ofd_array_add_string(result_html, Ofd_String_("<div class='ofd-image-title'>"));
                            ofd_parse_markdown(image_title.data, image_title.data + image_title.count, result_html, result_sections, link_references, next_section_id, log_data);
                            ofd_array_add_string(result_html, Ofd_String_("</div>"));
                        }
                        
                        ofd_array_add_string(result_html, Ofd_String_("</div>"));
                        // Output HTML. END
                        
                        c = tmp_c;
                        while(c < limit)
                        { // Go to the next line.
                            if(*c != '\n' && *c != '\r') break;
                            c++;
                        }
                        
                        break;
                    }
                    // Handle an image. END
                }
                
                goto do_the_default_thing;
            } break;
            
            
            case '|':
            {
                // Handle a table. START
                OFD_SPILL_TEXT();
                
                ofd_array_add_string(result_html, Ofd_String_("<table class='ofd-table'>"));
                
                while(c < limit)
                {
                    ofd_array_add_string(result_html, Ofd_String_("<tr>"));
                    
                    while(c < limit)
                    {
                        // Output cells. START
                        c++; // Skip '|'.
                        c = ofd_skip_whitespace(c, limit);
                        
                        ofd_array_add_string(result_html, Ofd_String_("<td class='ofd-cell'>"));
                        
                        // Get the cell limit.
                        char* end_of_line = NULL;
                        char* cell_limit = ofd_get_next_character_like_in_line(c, limit, '|', &end_of_line);
                        if(!cell_limit)
                        {
                            if(end_of_line) cell_limit = end_of_line;
                            else            cell_limit = limit;
                        }
                        //////////////////////
                        
                        
                        // Get the cell's content limit.
                        char* cell_content_limit = cell_limit - 1;
                        while(cell_content_limit >= lower_limit)
                        {
                            char character = *cell_content_limit;
                            if(character != ' ' && character != '\t') break;
                            
                            cell_content_limit--;
                        }
                        
                        cell_content_limit++;
                        ////////////////////////////////
                        
                        ofd_parse_markdown(c, cell_content_limit, result_html, result_sections, link_references, next_section_id, log_data);
                        ofd_array_add_string(result_html, Ofd_String_("</td>"));
                        
                        c = cell_limit;
                        if(cell_limit == limit || *cell_limit == '\n' || *cell_limit == '\r') break;
                        // Output cells. END
                    }
                    
                    ofd_array_add_string(result_html, Ofd_String_("</tr>"));
                    
                    if(c == limit) break;
                    
                    // Go to the next line.
                    if(*c == '\n')
                    {
                        c++;
                        if(c < limit && *c == '\r') c++;
                    }
                    else if(*c == '\r') c++;
                    ///////////////////////
                    
                    char* tmp_c = c;
                    tmp_c = ofd_skip_whitespace(tmp_c, limit);
                    
                    if(tmp_c == limit) break;
                    if(*tmp_c != '|') break; // The table stops here.
                    
                    c = tmp_c;
                }
                
                ofd_array_add_string(result_html, Ofd_String_("</table>"));
                // Handle a table. END
            } break;
            
            
            case '\\':
            {
                // Handle an escaped character. START
                OFD_SPILL_TEXT();
                
                c++;
                if(c < limit)
                {
                    if(*c == '\n')
                    {
                        c++;
                        if(c < limit && *c == '\r') c++;
                    }
                    else if(*c == '\r') c++;
                    
                    if(c < limit)
                    {
                        basic_text.data  = c;
                        basic_text.count = 1;
                        c++;
                    }
                }
                // Handle an escaped character. END
            } break;
            
            
            default:
            {
                do_the_default_thing:;
                
                if(!basic_text.count) basic_text.data = c;
                
                basic_text.count++;
                c++;
            } break;
        }
    }
    
    OFD_SPILL_TEXT();
}



ofd_static ofd_b8 ofd_parse_fields(char* data, int data_size, Ofd_Field* fields, int num_fields, char* error_message)
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE: on success this returns ofd_true and ofd_false otherwise. Error messages are written to error_message.
    // Note that the resulting theme is not initialised here, which means you can provide a default theme and only fields specified in theme_data
    // will be changed.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    error_message[0]  = 0;
    int parsed_fields = 0;
    
    char local_error_message[256];
    
    char* c     = data;
    char* limit = data + data_size;
    while(c < limit)
    {
        c = ofd_skip_whitespace(c, limit);
        if(c >= limit) break;
        
        
        if(*c == '#')
        {
            we_need_to_handle_a_comment:;
            // Handle a comment. START
            while(c < limit)
            {
                if(*c == '\n')
                {
                    c++;
                    if(c < limit && *c == '\r') c++;
                    break;
                }
                if(*c == '\r')
                {
                    c++;
                    break;
                }
                
                c++;
            }
            // Handle a comment. END
            
            continue;
        }
        
        
        // Get field. START
        Ofd_String field_name = {c};
        while(c < limit)
        {
            char character = *c;
            if(character == ' ' || character == '\t' || character == '\n' || character == '\r') break;
            
            field_name.count++;
            c++;
        }
        // Get field. END
        
        
        if(field_name.count)
        {
            // Parse field. START
            ofd_b8 found_the_field = ofd_false;
            for(int field_index = 0; field_index < num_fields; field_index++)
            {
                Ofd_Field* field = fields + field_index;
                if(!ofd_string_matches(field_name, field->name)) continue;
                
                if(parsed_fields & field->id)
                {
                    ofd_sprintf(error_message, "It looks like you already specified field '%s'.", field->name);
                    return ofd_false;
                }
                
                
                c = ofd_skip_whitespace(c, limit);
                
                if(c < limit && *c == '#') goto we_need_to_handle_a_comment; // It looks like the value is commented out.
                
                
                parsed_fields |= field->id;
                local_error_message[0] = 0;
                
                switch(field->type)
                {
                    case OFD_FIELD_COLOUR:
                    {
                        // Parse a colour value. START
                        Ofd_Colour* result = field->data;
                        
                        int num_components_parsed = 0;
                        while(c < limit)
                        {
                            // Skip junk. START
                            while(c < limit)
                            {
                                char character = *c;
                                
                                if((character >= '0' && character <= '9') || character == '.') break;
                                if(character == '\n' || character == '\r' || character == '#') break;
                                c++;
                            }
                            
                            if(c == limit) break;
                            if(*c == '#') break; // The rest of the line is commented out.
                            if(*c == '\n' || *c == '\r') break; // We reached the end of the line, no need to go further.
                            // Skip junk. END
                            
                            
                            if(num_components_parsed == 4)
                            {
                                ofd_sprintf(local_error_message, "It looks like you are trying to specify too many components.");
                                c = NULL;
                                break;
                            }
                            
                            
                            // Get component value. START
                            Ofd_String value = {c};
                            while(c < limit)
                            {
                                char character = *c;
                                if(!(character >= '0' && character <= '9') && character != '.') break;
                                
                                value.count++;
                                c++;
                            }
                            
                            char buffer[32];
                            ofd_to_c_string(value, buffer);
                            ofd_f32 v = ofd_to_f32(buffer);
                            
                            if(v < 0.0f || v > 1.0f)
                            {
                                ofd_sprintf(local_error_message, "Colour component values must be included in [0;1]. Got %g instead.", v);
                                c = NULL;
                                break;
                            }
                            
                            ofd_cast(result, ofd_f32*)[num_components_parsed] = v;
                            num_components_parsed++;
                            // Get component value. END
                        }
                        // Parse a colour value. END
                    } break;
                    
                    case OFD_FIELD_STRING:
                    {
                        // Parse a string value. START
                        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                        // NOTE: @ for now escape sequences (e.g. \<character>) are *not* supported. The reason for this is that we do not want to deal with
                        // allocations here, we just want to take a string view. We should support this at some point though.
                        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                        
                        Ofd_String* result = field->data;
                        result->count = 0;
                        
                        if(c == limit) break;
                        
                        
                        if(*c != '"')
                        {
                            ofd_sprintf(local_error_message, "Make sure to enclose a string value within double quotes.");
                            c = NULL;
                            break;
                        }
                        
                        c++;
                        result->data = c;
                        while(c < limit)
                        {
                            char character = *c;
                            if(character == '"' || character == '\n' || character == '\r') break;
                            
                            result->count++;
                            c++;
                        }
                        
                        if(c == limit || *c != '"')
                        {
                            ofd_printf(local_error_message, "Did you forget to close the string with a '\"'?");
                            c = NULL;
                            break;
                        }
                        
                        c++;
                        // Parse a string value. END
                    } break;
                    
                    case OFD_FIELD_F32:
                    {
                        // Parse a real number. START
                        ofd_f32* result = field->data;
                        
                        Ofd_String value = {c};
                        while(c < limit)
                        {
                            char character = *c;
                            if(character != '.' && (character < '0' || character > '9')) break;
                            
                            value.count++;
                            c++;
                        }
                        
                        if(!value.count)
                        {
                            if(c != limit && *c != '\n' && *c != '\r') ofd_sprintf(local_error_message, "It looks like this value is not a real number.");
                            break;
                        }
                        
                        char buffer[32];
                        ofd_to_c_string(value, buffer);
                        
                        *result = ofd_to_f32(buffer);
                        // Parse a real number. END
                    } break;
                    
                    default:
                    {
                        ofd_printf(error_message, "Field '%s' has an unknown type (%u).", field->type);
                        return ofd_false;
                    } break;
                }
                
                if(!c)
                {
                    ofd_sprintf(error_message, "Error parsing the value for '%s':\n%s", field->name, local_error_message);
                    return ofd_false;
                }
                
                
                while(c < limit)
                { // Go to the end of the line.
                    if(*c == '\n' || *c == '\r') break;
                    c++;
                }
                
                
                found_the_field = ofd_true;
                break;
            }
            
            
            if(!found_the_field)
            {
                char field_c_string[64];
                ofd_to_c_string(field_name, field_c_string);
                ofd_sprintf(error_message, "Field '%s' is unknown.", field_c_string);
                
                return ofd_false; // This is an unknown field.
            }
            // Parse field. END
        }
        
        
        // Skip new lines.
        while(c < limit)
        {
            if(*c != '\n' && *c !='\r') break;
            c++;
        }
        //////////////////
    }
    
    
    return ofd_true;
}

ofd_static ofd_b8 ofd_parse_theme_data(char* theme_data, int theme_data_size, Ofd_Theme* result, char* error_message)
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NOTE: on success this returns ofd_true and ofd_false otherwise. Error messages are written to error_message.
    // Note that the resulting theme is not initialised here, which means you can provide a default theme and only fields specified in theme_data
    // will be changed.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    const int OFD_BACKGROUND_COLOUR              = 0x01;
    const int OFD_TEXT_COLOUR                    = 0x02;
    const int OFD_CODE_BLOCK_BACKGROUND_COLOUR   = 0x04;
    const int OFD_CODE_BLOCK_TEXT_COLOUR         = 0x08;
    const int OFD_LINK_COLOUR                    = 0x10;
    const int OFD_IMAGE_TITLE_COLOUR             = 0x20;
    const int OFD_HORIZONTAL_RULE_COLOUR         = 0x40;
    const int OFD_QUOTE_BACKGROUND_COLOUR        = 0x80;
    const int OFD_QUOTE_TEXT_COLOUR              = 0x100;
    const int OFD_SECTION_LINK_TEXT_COLOUR       = 0x200;
    const int OFD_SECTION_LINK_BACKGROUND_COLOUR = 0x400;
    const int OFD_SEPARATOR_COLOUR               = 0x800;
    const int OFD_SCROLLBAR_COLOUR               = 0x1000;
    const int OFD_FONT_SIZE                      = 0x2000;
    
    const int OFD_SYNTAX_COMMENT_COLOUR   = 0x4000;
    const int OFD_SYNTAX_STRING_COLOUR    = 0x8000;
    const int OFD_SYNTAX_NUMBER_COLOUR    = 0x10000;
    const int OFD_SYNTAX_STRUCTURE_COLOUR = 0x20000;
    const int OFD_SYNTAX_KEYWORD_COLOUR   = 0x40000;
    const int OFD_SYNTAX_DIRECTIVE_COLOUR = 0x80000;
    
    Ofd_Field fields[] = {
        {OFD_BACKGROUND_COLOUR,              "background_colour",              &result->background_colour,              OFD_FIELD_COLOUR},
        {OFD_TEXT_COLOUR,                    "text_colour",                    &result->text_colour,                    OFD_FIELD_COLOUR},
        {OFD_CODE_BLOCK_BACKGROUND_COLOUR,   "code_block_background_colour",   &result->code_block_background_colour,   OFD_FIELD_COLOUR},
        {OFD_CODE_BLOCK_TEXT_COLOUR,         "code_block_text_colour",         &result->code_block_text_colour,         OFD_FIELD_COLOUR},
        {OFD_LINK_COLOUR,                    "link_colour",                    &result->link_colour,                    OFD_FIELD_COLOUR},
        {OFD_IMAGE_TITLE_COLOUR,             "image_title_colour",             &result->image_title_colour,             OFD_FIELD_COLOUR},
        {OFD_HORIZONTAL_RULE_COLOUR,         "horizontal_rule_colour",         &result->horizontal_rule_colour,         OFD_FIELD_COLOUR},
        {OFD_QUOTE_BACKGROUND_COLOUR,        "quote_background_colour",        &result->quote_background_colour,        OFD_FIELD_COLOUR},
        {OFD_QUOTE_TEXT_COLOUR,              "quote_text_colour",              &result->quote_text_colour,              OFD_FIELD_COLOUR},
        {OFD_SECTION_LINK_TEXT_COLOUR,       "section_link_text_colour",       &result->section_link_text_colour,       OFD_FIELD_COLOUR},
        {OFD_SECTION_LINK_BACKGROUND_COLOUR, "section_link_background_colour", &result->section_link_background_colour, OFD_FIELD_COLOUR},
        {OFD_SEPARATOR_COLOUR,               "separator_colour",               &result->separator_colour,               OFD_FIELD_COLOUR},
        {OFD_SCROLLBAR_COLOUR,               "scrollbar_colour",               &result->scrollbar_colour,               OFD_FIELD_COLOUR},
        {OFD_FONT_SIZE,                      "font_size",                      &result->font_size,                      OFD_FIELD_F32},
        
        {OFD_SYNTAX_COMMENT_COLOUR,          "syntax_comment_colour",          &result->syntax_comment_colour,          OFD_FIELD_COLOUR},
        {OFD_SYNTAX_STRING_COLOUR,           "syntax_string_colour",           &result->syntax_string_colour,           OFD_FIELD_COLOUR},
        {OFD_SYNTAX_NUMBER_COLOUR,           "syntax_number_colour",           &result->syntax_number_colour,           OFD_FIELD_COLOUR},
        {OFD_SYNTAX_STRUCTURE_COLOUR,        "syntax_structure_colour",        &result->syntax_structure_colour,        OFD_FIELD_COLOUR},
        {OFD_SYNTAX_KEYWORD_COLOUR,          "syntax_keyword_colour",          &result->syntax_keyword_colour,          OFD_FIELD_COLOUR},
        {OFD_SYNTAX_DIRECTIVE_COLOUR,        "syntax_directive_colour",        &result->syntax_directive_colour,        OFD_FIELD_COLOUR},
    };
    
    
    return ofd_parse_fields(theme_data, theme_data_size, fields, ofd_array_count(fields), error_message);
}


ofd_static ofd_b8 ofd_parse_configuration_data(char* data, int data_size, Ofd_Config* result, char* error_message)
{
    const int OUTPUT_FIELD = 0x01;
    const int THEME_FIELD  = 0x02;
    const int LOGO_FIELD   = 0x04;
    const int ICON_FIELD   = 0x08;
    const int TITLE_FIELD  = 0x10;
    
    Ofd_Field fields[] = {
        {OUTPUT_FIELD, "output", &result->output_filepath, OFD_FIELD_STRING},
        {THEME_FIELD,  "theme",  &result->theme_filepath,  OFD_FIELD_STRING},
        {LOGO_FIELD,   "logo",   &result->logo_path,       OFD_FIELD_STRING},
        {ICON_FIELD,   "icon",   &result->icon_path,       OFD_FIELD_STRING},
        {TITLE_FIELD,  "title",  &result->title,           OFD_FIELD_STRING},
    };
    
    return ofd_parse_fields(data, data_size, fields, ofd_array_count(fields), error_message);
}


ofd_static void ofd_generate_documentation_from_memory(Ofd_String* markdown_files, char** markdown_filepaths, int num_markdown_files, Ofd_Array* output, Ofd_Theme theme, char* logo_path, char* icon_path, char* title, Ofd_Array* log_data)
{
    //////////////////////////////////////////////////////////////
    // NOTE: it is up to you to free output with ofd_free_array().
    //////////////////////////////////////////////////////////////
    
    char buffer[2048];
    char colour_string[64];
    char colour_string2[64];
    char colour_string3[64];
    
    
    Ofd_Array html, sections;
    
    ofd_array_init(&html,     1);
    ofd_array_init(&sections, sizeof(Ofd_Section));
    ofd_array_init(log_data, 1);
    
    
    int next_section_id = 0;
    
    // Add basic HTML stuff. START
    ofd_array_add_string(&html, Ofd_String_(
        "<!doctype html>"
        "<html>"
        "<head>"
            "<meta charset='utf-8'>"
            "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    ));
    
    if(title)
    {
        ofd_sprintf(buffer, "<title>%s</title>", title);
        ofd_array_add_string(&html, Ofd_String_(buffer));
    }
    if(icon_path)
    {
        ofd_sprintf(buffer,
            "<link rel='icon' href='%s' />"
            "<link rel='apple-touch-icon' href='%s' />"
            
            "<meta name='twitter:card' content='summary'>"
            "<meta property='og:type' content='website'>"
            "<meta property='og:title' content='%s'>"
            "<meta property='og:image' content='%s'>"
            ,
            icon_path, icon_path,
            title, icon_path
        );
        ofd_array_add_string(&html, Ofd_String_(buffer));
    }
    
    ofd_array_add_string(&html, Ofd_String_(
        "</head>"
        "<body>"
    ));
    // Add basic HTML stuff. END
    
    
    ofd_f32 base_font_size = theme.font_size; // In percentage of the render target height.
    
    ofd_array_add_string(&html, Ofd_String_("<style>"));
    
    
    // Generate basic style. START
    ofd_colour_to_string(theme.background_colour, colour_string);
    ofd_colour_to_string(theme.text_colour,       colour_string2);
    ofd_colour_to_string(theme.separator_colour,  colour_string3);
    
    ofd_f32 ofd_link_height = base_font_size * 3.0f;
    ofd_f32 logo_height     = base_font_size * 3.0f;
    ofd_f32 sidebar_width   = base_font_size * 15.0f;
    ofd_f32 sidebar_height  = 100.0f - (ofd_link_height + logo_height);
    
    ofd_sprintf(buffer,
        "html {"
            "white-space: pre-wrap;"
            "font-family: system-ui, system-ui, ui-sans-serif, sans-serif;"
            "font-size: %gvh;"
            "background-color: %s;"
            "color: %s;"
            "overflow-x: clip;"
        "}"
        
        "#ofd-logo-container {"
            "position: fixed;"
            "left: 0;"
            "top: 0;"
            "width: 100vw;"
            "height: calc(%gvh - 1px);" // '- 1px' so that we can put a 1px separator between the logo and the sidebar.
        "}"
        
        "#ofd-sidebar {"
            "position: fixed;"
            "left: 0;"
            "top: %gvh;"
            "width: calc(%gvh - 1px);" // '- 1px' so that we can put a 1px separator between the sidebar and the main content.
            "height: %gvh;"
            "overflow-y: auto;"
            "overflow-x: clip;"
            "overflow-wrap: break-word;"
        "}"
        
        "#ofd-repo-link {"
            "position: fixed;"
            "left: 0;"
            "top: %gvh;"
            "width: %gvh;"
            "height: %gvh;"
            "overflow-wrap: break-word;"
            "opacity: 0.8;"
        "}"
        
        "#ofd-repo-link:hover {"
            "opacity: 1;"
        "}"
        
        "#ofd-sidebar-separator {"
            "position: fixed;"
            "top: %gvh;"
            "left: calc(%gvh - 1px);"
            "width: 1px;"
            "height: %gvh;"
            "background-color: %s;"
        "}"
        
        "#ofd-logo-separator {"
            "position: fixed;"
            "top: calc(%gvh - 1px);"
            "left: 0;"
            "width: 100vw;"
            "height: 1px;"
            "background-color: %s;"
        "}"
        
        "#ofd-main-content {"
            "position: fixed;"
            "left: %gvh;"
            "top: %gvh;"
            "width: calc(98%% - %gvh);"
            "height: %gvh;"
            "padding-left: 1%%;"
            "padding-right: 1%%;"
            "overflow-y: scroll;"
            "overflow-x: clip;"
            "overflow-wrap: break-word;"
            
            "transition: all 0.2s;"
            "-webkit-transition: all 0.2s;"
        "}"
        
        "#ofd-sidebar-toggle {"
            "display: block;"
            "position: fixed;"
            "top: 0;"
            "left: 0;"
            "width: %gvh;"
            "height: %gvh;"
        "}"
        ,
        base_font_size, colour_string, colour_string2, // html
        logo_height, // #ofd-logo-container
        logo_height, sidebar_width, sidebar_height, // #ofd-sidebar
        logo_height + sidebar_height, sidebar_width, ofd_link_height, // #ofd-repo-link
        logo_height, sidebar_width, 100.0f - logo_height, colour_string3, // #ofd-sidebar-separator
        logo_height, colour_string3, // #ofd-logo-separator
        sidebar_width, logo_height, sidebar_width, 100.0f - logo_height, // #ofd-main-content
        logo_height, logo_height // #ofd-sidebar-toggle
    );
    
    ofd_array_add_string(&html, Ofd_String_(buffer));
    
    ofd_colour_to_string(theme.text_colour, colour_string);
    
    ofd_f32 menu_width  = logo_height * 0.6f;
    ofd_f32 menu_height = logo_height * 0.05f;
    
    ofd_sprintf(buffer,
        "#ofd-sidebar-checkbox:not(:checked) ~ .ofd-sidebar-element {"
            "transform: translateX(-%gvh);"
            "-webkit-transform: translateX(-%gvh);"
            
            "transition: transform 0.2s;"
            "-webkit-transition: transform 0.2s;"
        "}"
        
        "#ofd-sidebar-checkbox:not(:checked) ~ #ofd-main-content {"
            "transform: translateX(-%gvh);"
            "-webkit-transform: translateX(-%gvh);"
            "width: calc(100vw - 2%%);"
            
            "transition: all 0.2s;"
            "-webkit-transition: all 0.2s;"
        "}"
        
        "#ofd-menu-part {"
            "display: block;"
            "position: fixed;"
            "top: %gvh;"
            "left: %gvh;"
            "width: %gvh;"
            "height: %gvh;"
            "background-color: %s;"
            "transform-origin: center;"
            "-webkit-transform-origin: center;"
            
            "transition: all 0.2s;"
            "-webkit-transition: all 0.2s;"
        "}"
        
        "#ofd-menu-part:before, #ofd-menu-part:after {"
            "display: block;"
            "position: absolute;"
            "content: \"\";"
            "width: 100%%;"
            "height: 100%%;"
            "background-color: %s;"
            
            "transition: all 0.2s;"
            "-webkit-transition: all 0.2s;"
        "}"
        ,
        sidebar_width, sidebar_width,
        sidebar_width, sidebar_width,
        logo_height * 0.5f - menu_height * 0.5f, logo_height * 0.5f - menu_width * 0.5f, menu_width, menu_height, colour_string,
        colour_string
    );
    
    ofd_array_add_string(&html, Ofd_String_(buffer));
    // Generate basic style. END
    
    
    // Generate scrollbar style. START
    Ofd_Colour scrollbar_bg_colour = theme.scrollbar_colour;
    if(scrollbar_bg_colour.a >= 0.5f) scrollbar_bg_colour.a *= 0.6f;
    
    ofd_colour_to_string(theme.scrollbar_colour, colour_string);
    ofd_colour_to_string(scrollbar_bg_colour,    colour_string2);
    
    ofd_sprintf(buffer, 
        "#ofd-main-content::-webkit-scrollbar, #ofd-sidebar::-webkit-scrollbar {"
            "width: 1.8vh;"
        "}"
        
        "#ofd-main-content::-webkit-scrollbar-track, #ofd-sidebar::-webkit-scrollbar-track {"
            "background-color: #00000000;"
        "}"
        
        "#ofd-main-content::-webkit-scrollbar-track:hover, #ofd-sidebar::-webkit-scrollbar-track:hover {"
            "background-color: %s;"
        "}"
        
        "#ofd-main-content::-webkit-scrollbar-thumb, #ofd-sidebar::-webkit-scrollbar-thumb {"
            "background-color: %s;"
            "width: 0vh;"
        "}"
        ,
        colour_string2, colour_string
    );
    
    ofd_array_add_string(&html, Ofd_String_(buffer));
    // Generate scrollbar style. END
    
    
    // Generate header styles. START
    ofd_f32 max_font_size = base_font_size * 2.0f;
    ofd_f32 min_font_size = base_font_size * 0.7f;
    ofd_f32 font_size_gap = max_font_size - min_font_size;
    for(ofd_u8 header_hierarchy = 1; header_hierarchy <= OFD_MAX_HEADER_LEVEL; header_hierarchy++)
    {
        ofd_f32 lerp_factor = ofd_cast(header_hierarchy - 1, ofd_f32) / ofd_cast(OFD_MAX_HEADER_LEVEL, ofd_f32);
        lerp_factor = 1.0f - lerp_factor;
        lerp_factor *= lerp_factor;
        
        ofd_f32 font_size   = max_font_size * lerp_factor + min_font_size * (1.0f - lerp_factor);
        
        ofd_f32 margin      = font_size * 0.5f;
        
        ofd_sprintf(buffer,
            ".ofd-header-%u {"
                "font-size: %gvh;"
                "margin-bottom: %gvh;"
            "}"
            ,
            header_hierarchy, font_size, margin
        );
        
        ofd_array_add_string(&html, Ofd_String_(buffer));
    }
    // Generate header styles. END
    
    
    // Generate code block styles. START
    ofd_colour_to_string(theme.code_block_background_colour, colour_string);
    ofd_colour_to_string(theme.code_block_text_colour,       colour_string2);
    
    ofd_sprintf(buffer,
        ".ofd-code-block {"
            "font-family: monospace, monospace;"
            "background-color: %s;"
            "color: %s;"
        "}"
        
        ".ofd-code-block-0 {"
            "display: block;"
            "border-radius: %gvh;"
            "padding: %gvh;"
        "}"
        
        ".ofd-code-block-1 {"
            "display: inline;"
            "border-radius: %gvh;"
            "padding: %gvh;"
            "padding-left: %gvh;"
            "padding-right: %gvh;"
        "}"
        ,
        colour_string, colour_string2,
        base_font_size * 0.32f, base_font_size * 0.35f,
        base_font_size * 0.25f, base_font_size * 0.05f, base_font_size * 0.2f, base_font_size * 0.2f
    );
    
    ofd_array_add_string(&html, Ofd_String_(buffer));
    // Generate code block styles. END
    
    
    // Generate link style. START
    ofd_colour_to_string(theme.link_colour, colour_string);
    
    ofd_sprintf(buffer,
        ".ofd-external-link {"
            "color: %s;"
        "}"
        ,
        colour_string
    );
    
    ofd_array_add_string(&html, Ofd_String_(buffer));
    // Generate link style. END
    
    
    // Generate image title style. START
    ofd_colour_to_string(theme.image_title_colour, colour_string);
    
    ofd_sprintf(buffer,
        ".ofd-image-title {"
            "display: block;"
            "color: %s;"
        "}"
        ,
        colour_string
    );
    
    ofd_array_add_string(&html, Ofd_String_(buffer));
    // Generate image title style. END
    
    
    // Generate horizontal rule style. START
    ofd_colour_to_string(theme.horizontal_rule_colour, colour_string);
    ofd_colour_to_string(theme.separator_colour,       colour_string2);
    
    ofd_f32 rule2_margin = base_font_size * 0.5f;
    
    ofd_sprintf(buffer,
        ".ofd-horizontal-rule {"
            "display: block;"
            "width: 100%%;"
            "min-height: 2px;"
            "background-color: %s;"
            "border-radius: 100vh;"
        "}"
        
        ".ofd-horizontal-rule-2 {"
            "display: block;"
            "width: 90%%;"
            "min-height: 2px;"
            "margin-left: auto;"
            "margin-right: auto;"
            "margin-top: %gvh;"
            "margin-bottom: %gvh;"
            "background-color: %s;"
            "border-radius: 100vh;"
        "}"
        ,
        colour_string,
        rule2_margin, rule2_margin, colour_string2
    );
    
    ofd_array_add_string(&html, Ofd_String_(buffer));
    // Generate horizontal rule style. END
    
    
    // Generate quote style. START
    ofd_colour_to_string(theme.quote_background_colour, colour_string);
    ofd_colour_to_string(theme.quote_text_colour,       colour_string2);
    
    ofd_sprintf(buffer,
        ".ofd-quote {"
            "display: block;"
            "margin-left: 3%%;"
            "margin-right: 3%%;"
            "background-color: %s;"
            "color: %s;"
            "padding: %gvh;"
        "}"
        ,
        colour_string, colour_string2, base_font_size
    );
    
    ofd_array_add_string(&html, Ofd_String_(buffer));
    // Generate quote style. END
    
    
    // Generate section link styles. START
    ofd_colour_to_string(theme.section_link_text_colour, colour_string);
    
    ofd_sprintf(buffer,
        ".ofd-section-link-0 {"
            "display: inline;"
            "text-transform: uppercase;"
            "font-weight: bold;"
            "color: %s;"
        "}"
        
        ".ofd-section-link-1 {"
            "display: inline-block;"
            "color: %s;"
        "}"
        ,
        colour_string,
        colour_string
    );
    ofd_array_add_string(&html, Ofd_String_(buffer));
    
    
    Ofd_Colour subsection_link_colour = theme.section_link_text_colour;
    subsection_link_colour.a *= 0.7f;
    
    ofd_colour_to_string(subsection_link_colour, colour_string);
    
    for(int i = 1; i < OFD_MAX_HEADER_LEVEL; i++)
    {
        ofd_sprintf(buffer,
            ".ofd-section-link-%d {"
                "display: inline-block;"
                "padding-left: %g%%;"
                "color: %s;"
            "}"
            ,
            i + 1, ofd_cast(i - 1, ofd_f32) * 5.0f, colour_string
        );
        
        ofd_array_add_string(&html, Ofd_String_(buffer));
    }
    
    
    Ofd_Colour subsection_rule_colour = theme.section_link_text_colour;
    subsection_rule_colour.a *= 0.5f;
    
    ofd_colour_to_string(subsection_rule_colour, colour_string);
    
    ofd_sprintf(buffer,
        ".ofd-subsection {"
            "position: relative;"
            "top: 0;"
            "left: 0;"
            "padding-left: 5%%;"
        "}"
        
        ".ofd-subsection-rule {"
            "position: absolute;"
            "top: 0;"
            "left: 4%%;"
            "height: 100%%;"
            "width: 2px;"
            "border-radius: 100vh;"
            "background-color: %s;"
        "}"
        ,
        colour_string
    );
    
    ofd_array_add_string(&html, Ofd_String_(buffer));
    
    
    ofd_colour_to_string(theme.section_link_background_colour, colour_string);
    ofd_colour_to_string(theme.text_colour,                    colour_string2);
    
    ofd_f32 dropdown_arrow_size = base_font_size * 0.5f;
    
    ofd_sprintf(buffer, 
        ".ofd-section-link:hover {"
            "background-color: %s;"
            "cursor: pointer;"
            
            "transition: background-color 0.15s;"
            "-webkit-transition: background-color 0.15s;"
        "}"
        
        ".ofd-dropdown-arrow {"
            "position: absolute;"
            "top: 50%%;"
            "left: 50%%;"
            "width: 0;"
            "height: 0;"
            
            "border-left: %gvh solid transparent;"
            "border-right: %gvh solid transparent;"
            "border-top: %gvh solid %s;"
            
            "transform: translateY(-50%%);"
            "-webkit-transform: translateY(-50%%);"
        "}"
        ,
        colour_string,
        dropdown_arrow_size, dropdown_arrow_size, dropdown_arrow_size, colour_string2
    );
    
    ofd_array_add_string(&html, Ofd_String_(buffer));
    // Generate section link styles. END
    
    
    // Generate table style. START
    ofd_colour_to_string(theme.text_colour, colour_string);
    
    ofd_sprintf(buffer,
        ".ofd-cell {"
            "border: 1px solid %s;"
            "padding: 5px;"
            "padding-left: 10px;"
            "padding-right: 10px;"
        "}"
        ,
        colour_string
    );
    
    ofd_array_add_string(&html, Ofd_String_(buffer));
    // Generate table style. END
    
    
    // Generate syntax styles. START
    ofd_colour_to_string(theme.syntax_comment_colour, colour_string);
    ofd_colour_to_string(theme.syntax_string_colour,  colour_string2);
    ofd_colour_to_string(theme.syntax_number_colour,  colour_string3);
    
    ofd_sprintf(buffer,
        ".ofd-syntax-comment {"
            "color: %s;"
        "}"
        
        ".ofd-syntax-string {"
            "color: %s;"
        "}"
        
        ".ofd-syntax-number {"
            "color: %s;"
        "}"
        ,
        colour_string,
        colour_string2,
        colour_string3
    );
    
    ofd_array_add_string(&html, Ofd_String_(buffer));
    
    
    ofd_colour_to_string(theme.syntax_structure_colour, colour_string);
    ofd_colour_to_string(theme.syntax_keyword_colour,   colour_string2);
    ofd_colour_to_string(theme.syntax_directive_colour, colour_string3);
    
    ofd_sprintf(buffer,
        ".ofd-syntax-structure {"
            "color: %s;"
        "}"
        
        ".ofd-syntax-keyword {"
            "color: %s;"
        "}"
        
        ".ofd-syntax-directive {"
            "color: %s;"
        "}"
        ,
        colour_string,
        colour_string2,
        colour_string3
    );
    
    ofd_array_add_string(&html, Ofd_String_(buffer));
    // Generate syntax styles. END
    
    
    ofd_array_add_string(&html, Ofd_String_(
        "#ofd-menu-part:before {"
            "top: -300%;"
        "}"
        
        "#ofd-menu-part:after {"
            "bottom: -300%;"
        "}"
        
        "#ofd-sidebar-checkbox:checked + #ofd-logo-container #ofd-sidebar-toggle #ofd-menu-part {"
            "transform: rotate(45deg);"
            "-webkit-transform: rotate(45deg);"
            
            "transition: all 0.2s;"
            "-webkit-transition: all 0.2s;"
        "}"
        
        "#ofd-sidebar-checkbox:checked + #ofd-logo-container #ofd-sidebar-toggle #ofd-menu-part:after {"
            "transform: rotate(90deg);"
            "-webkit-transform: rotate(90deg);"
            "bottom: 0;"
            
            "transition: all 0.2s;"
            "-webkit-transition: all 0.2s;"
        "}"
        
        "#ofd-sidebar-checkbox:checked + #ofd-logo-container #ofd-sidebar-toggle #ofd-menu-part:before {"
            "transform: rotate(90deg);"
            "-webkit-transform: rotate(90deg);"
            "top: 0;"
            
            "transition: all 0.2s;"
            "-webkit-transition: all 0.2s;"
        "}"
        
        "#ofd-logo {"
            "display: block;"
            "margin: 0 auto;"
            "max-width: 90%;"
            "max-height: 100%;"
        "}"
        
        
        ".ofd-sidebar-element {"
            "transition: transform 0.2s;"
            "-webkit-transition: transform 0.2s;"
        "}"
        
        
        ".ofd-syntax {"
            "display: inline;"
        "}"
        
        
        ".ofd-table {"
            "margin: 0 auto;"
            "border-collapse: collapse;"
        "}"
        
        ".ofd-header {"
            "display: block;"
            "font-weight: bold;"
        "}"
        
        ".ofd-section-link-wrapper {"
            "position: relative;"
            "top: 0;"
            "left: 0;"
            "width: 100%;"
        "}"
        
        ".ofd-dropdown-button {"
            "position: absolute;"
            "top: 0;"
            "left: 80%;"
            "width: 20%;"
            "height: 100%;"
        "}"
        
        ".ofd-section-link {"
            "display: inline-block;"
            "text-decoration: none;"
            "background-color: #00000000;"
            "border-radius: 2px;"
            "margin-left: 1%;"
            "margin-right: 1%;"
            "margin-bottom: 2px;"
            "padding: 2%;"
            
            "transition: background-color 0.15s;"
            "-webkit-transition: background-color 0.15s;"
        "}"
        
        
        ".ofd-code-block .ofd-external-link {"
            "color: inherit;"
        "}"
        
        ".ofd-quote-line {"
            "display: block;"
        "}"
        
        ".ofd-quote .ofd-code-block, .ofd-quote .ofd-external-link, .ofd-quote .ofd-image-title {"
            "color: inherit;"
        "}"
        
        ".ofd-strike-through {"
            "display: inline;"
            "text-decoration-line: line-through;"
        "}"
        
        ".ofd-list-item {"
            "display: list-item;"
            "list-style-position: inside;"
            "list-style-type: disc;"
        "}"
        
        ".ofd-image-container {"
            "display: block;"
            "width: 100%;"
            "text-align: center;"
        "}"
        
        ".ofd-basic-image {"
            "margin: 0 auto;"
            "max-width: 100%;"
        "}"
        
        ".ofd-emphasis-1 {"
            "display: inline;"
            "font-style: italic;"
        "}"
        
        ".ofd-emphasis-2 {"
            "display: inline;"
            "font-weight: bold;"
        "}"
        
        ".ofd-anchor {"
            "display: block;"
            "width: 0;"
            "height: 0;"
        "}"
        "</style>"
        
        "<input type='checkbox' id='ofd-sidebar-checkbox' style='display: none;' checked/>"
    ));
    
    
    // Parse markdown files. START
    Ofd_Array md_html, link_references;
    ofd_array_init(&md_html, 1);
    ofd_array_init(&link_references, sizeof(Ofd_Link_Reference));
    
    for(int file_index = 0; file_index < num_markdown_files; file_index++)
    {
        if(file_index > 0) ofd_array_add_string(&md_html, Ofd_String_("<br/><br/><div class='ofd-horizontal-rule'></div><br/>"));
        
        
        Ofd_String filename = Ofd_String_(markdown_filepaths[file_index]);
        filename = ofd_get_last_item_from_filepath(filename);
        filename = ofd_strip_file_extension(filename);
        filename.count = ofd_min(filename.count, OFD_MAX_SECTION_NAME_COUNT);
        
        Ofd_Section* section = ofd_array_add_fast(&sections);
        section->id         = next_section_id;
        section->hierarchy  = 0;
        
        filename = ofd_cleanup_section_name(filename, section->name_memory);
        section->name_count = filename.count;
        
        Ofd_String section_id = ofd_make_section_id(filename, section->id_memory);
        section->id_count = section_id.count;
        
        ofd_copy(section->full_id_memory, section->id_memory, section->id_count);
        section->full_id_count = section->id_count;
        
        
        ofd_array_add_string(&md_html, Ofd_String_("<div class='ofd-anchor' id='"));
        ofd_array_add_string(&md_html, section_id);
        ofd_array_add_string(&md_html, Ofd_String_("'></div>"));
        
        
        next_section_id++;
        
        
        Ofd_String* markdown_data = markdown_files + file_index;
        ofd_parse_markdown(markdown_data->data, markdown_data->data + markdown_data->count, &md_html, &sections, &link_references, &next_section_id, log_data);
    }
    
    
    // Free link references.
    for(int i = 0; i < link_references.count; i++)
    {
        Ofd_Link_Reference* ref = ofd_cast(link_references.data, Ofd_Link_Reference*) + i;
        ofd_free_array(&ref->additional_css);
    }
    ofd_free_array(&link_references);
    ////////////////////////
    // Parse markdown files. END
    
    
    // Add logo. START
    ofd_array_add_string(&html, Ofd_String_("<div id='ofd-logo-container'>"));
    
    ofd_array_add_string(&html, Ofd_String_(
            "<label for='ofd-sidebar-checkbox' id='ofd-sidebar-toggle'>"
                "<div id='ofd-menu-part'></div>"
            "</label>"
    ));
    
    if(logo_path)
    {
        ofd_sprintf(buffer, "<img id='ofd-logo' src='%s' />", logo_path);
        ofd_array_add_string(&html, Ofd_String_(buffer));
    }
    
    ofd_array_add_string(&html, Ofd_String_("</div>"));
    // Add logo. END
    
    
    // Add sidebar. START
    ofd_array_add_string(&html, Ofd_String_("<div id='ofd-sidebar' class='ofd-sidebar-element'>"));
    
    // Add sections. START
    Ofd_Section* previous_root_section = NULL;
    
    for(int section_index = 0; section_index < sections.count; section_index++)
    {
        Ofd_Section* section = ofd_cast(sections.data, Ofd_Section*) + section_index;
        
        if(section->hierarchy == 0 && section_index > 0)
        {
            if(previous_root_section)
            {
                if(previous_root_section != section - 1) ofd_array_add_string(&html, Ofd_String_("</div>"));
            }
            
            ofd_array_add_string(&html, Ofd_String_("<div class='ofd-horizontal-rule-2'></div>"));
            
            previous_root_section = NULL;
        }
        
        
        Ofd_String section_name = {section->name_memory, section->name_count};
        Ofd_String section_id   = {section->full_id_memory, section->full_id_count};
        
        if(previous_root_section)
        {
            if(section->hierarchy == 1)
            {
                if(previous_root_section != section - 1) ofd_array_add_string(&html, Ofd_String_("</div>"));
            }
        }
        
        
        ofd_b8 next_comes_a_subsection = ofd_false;
        if(section->hierarchy == 1)
        {
            previous_root_section = section;
            if(section_index + 1 < sections.count)
            {
                Ofd_Section* next_section = section + 1;
                next_comes_a_subsection = (next_section->hierarchy > 1);
                
                ofd_colour_to_string(theme.text_colour, colour_string);
                
                ofd_sprintf(buffer,
                    "<style>"
                    
                    "#ofd-dropdown-%d:not(:checked) ~ #ofd-subsection-%d {"
                        "display: none;"
                    "}"
                    
                    "#ofd-dropdown-%d:not(:checked) ~ div #ofd-dropdown-button-%d .ofd-dropdown-arrow {"
                        "border-top: %gvh solid transparent;"
                        "border-bottom: %gvh solid transparent;"
                        "border-left: %gvh solid %s;"
                        
                        "transform: translateY(-50%%);"
                        "-webkit-transform: translateY(-50%%);"
                    "}"
                    
                    "</style>"
                    ,
                    section->id, section->id,
                    section->id, section->id,
                    dropdown_arrow_size, dropdown_arrow_size, dropdown_arrow_size, colour_string
                );
                ofd_array_add_string(&html, Ofd_String_(buffer));
            }
        }
        
        
        if(next_comes_a_subsection)
        {
            ofd_sprintf(buffer, "<input type='checkbox' id='ofd-dropdown-%d' style='display: none;' />", section->id);
            ofd_array_add_string(&html, Ofd_String_(buffer));
        }
        
        ofd_f32 link_width = next_comes_a_subsection? 80 : 93;
        ofd_sprintf(buffer, "<div class='ofd-section-link-wrapper'><a class='ofd-section-link' style='width:%g%%;' href='#", link_width);
        ofd_array_add_string(&html, Ofd_String_(buffer));
        ofd_array_add_string(&html, section_id);
        
        ofd_sprintf(buffer, "'><div class='ofd-section-link-%d'>", section->hierarchy);
        ofd_array_add_string(&html, Ofd_String_(buffer));
        
        ofd_array_add_string(&html, section_name);
        
        ofd_array_add_string(&html, Ofd_String_("</div></a>"));
        
        if(next_comes_a_subsection)
        {
            ofd_sprintf(buffer, "<label for='ofd-dropdown-%d' class='ofd-dropdown-button' id='ofd-dropdown-button-%d'><div class='ofd-dropdown-arrow'></div></label></div>", section->id, section->id);
            ofd_array_add_string(&html, Ofd_String_(buffer));
            
            ofd_sprintf(buffer,
                "<div class='ofd-subsection' id='ofd-subsection-%d'><div class='ofd-subsection-rule'></div>"
                ,
                section->id
            );
            ofd_array_add_string(&html, Ofd_String_(buffer));
        }
        else ofd_array_add_string(&html, Ofd_String_("</div>")); // Closing the section link wrapper.
        
        if(section->hierarchy != 1 && section_index + 1 == sections.count && previous_root_section) ofd_array_add_string(&html, Ofd_String_("</div>"));
    }
    // Add sections. END
    
    ofd_array_add_string(&html, Ofd_String_("</div>"));
    
    
    ofd_array_add_string(&html, Ofd_String_("<div id='ofd-repo-link' class='ofd-sidebar-element'>"));
    
    ofd_f32 background_brightness = theme.background_colour.r * 0.2126f + theme.background_colour.g * 0.7152f + theme.background_colour.b * 0.0722f;
    char* logo_source = background_brightness < 0.5f? "https://deliciouslines.com/images/ofd-logo-white.png" : "https://deliciouslines.com/images/ofd-logo-black.png";
    ofd_sprintf(buffer,
        "<a href='https://github.com/deliciouslines/one-file-docs' target='_blank' style='display: inline-block; margin: 0; padding: 0; width: 100%%; height: 100%%;'>"
            "<img style='display: block; max-width: 100%%; max-height: 100%%; margin: auto;' src='%s' />"
        "</a>"
        ,
        logo_source
    );
    ofd_array_add_string(&html, Ofd_String_(buffer));
    
    ofd_array_add_string(&html, Ofd_String_("</div>"));
    
    
    ofd_array_add_string(&html, Ofd_String_("<div id='ofd-sidebar-separator' class='ofd-sidebar-element'></div>"));
    ofd_array_add_string(&html, Ofd_String_("<div id='ofd-logo-separator'></div>"));
    // Add sidebar. END
    
    // Add main content.
    ofd_array_add_string(&html, Ofd_String_("<div id='ofd-main-content'>"));
    ofd_array_add_array(&html, &md_html);
    
    ofd_free_array(&md_html);
    
    ofd_array_add_string(&html, Ofd_String_("<div style='display: block; height: 1vh;'></div></div>"));
    ////////////////////
    
    ofd_array_add_string(&html, Ofd_String_(
        "</html>"
        "</body>"
    ));
    
    ofd_free_array(&sections);
    
    
    *output = html;
    
    char* null_character = ofd_array_add_fast(log_data);
    *null_character = 0;
}

#ifndef OFD_NO_FILE_IO
ofd_static ofd_b8 ofd_generate_documentation(char** markdown_filepaths, int num_markdown_files, char* output_filepath, char* theme_filepath, char* logo_path, char* icon_path, char* title)
{
    #define OFD_GEN_DOCS_RETURN(_v)\
    {\
        ofd_free_array(&html);\
        ofd_free_array(&log_data);\
        \
        for(int md_index = 0; md_index < markdown_files.count; md_index++)\
        {\
            Ofd_String* data = ofd_cast(markdown_files.data, Ofd_String*) + md_index;\
            ofd_free(data->data);\
        }\
        \
        ofd_free_array(&markdown_files);\
        return _v;\
    }
    
    
    Ofd_Array html     = {0};
    Ofd_Array log_data = {0};
    
    
    // Load markdown files. START
    Ofd_Array markdown_files;
    ofd_array_init_with_space(&markdown_files, sizeof(Ofd_String), num_markdown_files);
    for(int file_index = 0; file_index < num_markdown_files; file_index++)
    {
        Ofd_String* markdown_data = ofd_array_add_fast(&markdown_files);
        if(!ofd_read_whole_file(markdown_filepaths[file_index], markdown_data))
        {
            OFD_GEN_DOCS_RETURN(ofd_false);
        }
    }
    // Load markdown files. END
    
    
    
    Ofd_Theme default_theme = {
        {0.06f, 0.08f, 0.1f, 1},      // Background colour.
        {0.8f,  0.8f,  0.8f, 1},      // Text colour.
        {0.6f,  0.8f,  1,    0.15f},  // Code block background colour.
        {0.8f,  0.8f,  0.8f, 1},      // Code block text colour.
        {0,     0.6f,  1,    1},      // Link colour.
        {1,     1,     1,    0.5f},   // Image title colour.
        {1,     1,     1,    0.3f},   // Horizontal rule colour.
        {0,     0.6f,  1,    1},      // Quote background colour.
        {0,     0,     0,    1},      // Quote text colour.
        {0.8f,  0.8f,  0.8f, 1},      // Section link text colour.
        {1,     1,     1,    0.2f},   // Section link background colour.
        {1,     1,     1,    0.3f},   // Separator colour.
        {1,     1,     1,    0.2f},   // Scrollbar colour.
        
        2,                            // Font size.
        
        {0.1f, 0.8f,  0.5f, 1},    // Comment colour.
        {0.2f, 0.7f,  1,    1},    // String colour.
        {0.2f, 0.7f,  1,    1},    // Number colour.
        {1,    1,     1,    0.6f}, // Structure colour.
        {1,    0.3f,  0.1f, 1},    // Keyword colour.
        {1,    1,     1,    1},    // Directive colour.
    };
    
    if(theme_filepath)
    {
        // Parse theme file. START
        Ofd_String theme_data;
        if(!ofd_read_whole_file(theme_filepath, &theme_data)) OFD_GEN_DOCS_RETURN(ofd_false);
        
        char error_message[512];
        ofd_b8 status = ofd_parse_theme_data(theme_data.data, theme_data.count, &default_theme, error_message);
        ofd_free(theme_data.data);
        
        if(!status)
        {
            ofd_printf("Failed to parse theme file '%s' with the following error:\n%s\n\n", theme_filepath, error_message);
            OFD_GEN_DOCS_RETURN(ofd_false);
        }
        // Parse theme file. END
    }
    
    
    ofd_generate_documentation_from_memory(ofd_cast(markdown_files.data, Ofd_String*), markdown_filepaths, markdown_files.count, &html, default_theme, logo_path, icon_path, title, &log_data);
    if(log_data.count > 1) ofd_printf("\n%s", log_data.data);
    
    ofd_b8 status = ofd_write_whole_file(output_filepath, html.data, html.count);
    
    OFD_GEN_DOCS_RETURN(status);
}
#endif // OFD_NO_FILE_IO

#ifdef __cplusplus
}
#endif
#endif // OFD_IMPLEMENTATION

/*
                            LICENSE (MIT)

Copyright (c) 2023 Delicious Lines

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/