#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <UI/UI.h>
#include <UI/UIElement.h>

int drop_operation ( UIWindow_t *p_window, char *path )
{

    // Initialized data
    UIInstance_t  *p_instance       = ui_get_active_instance();
    UITextInput_t *p_ai_path        = find_element(p_window, "ai tool path");
    size_t         len              = ui_load_file(path, 0, false);
    char          *text             = calloc(len+1, sizeof(char));
    JSONValue_t   *p_value          = 0,
                  *p_name           = 0,
                  *p_initial_state  = 0,
                  *p_states         = 0;
    UIElement_t   *name             = find_element(p_window, "ai name"),
                  *initial_state    = find_element(p_window, "initial state"),
                  *states           = find_element(p_window, "states table"),
                  *add_state        = find_element(p_window, "ai add state button"),
                  *remove_state     = find_element(p_window, "ai remove state button"),
                  *state_name_tinp  = find_element(p_window, "add state name text input"),
                  *initial_state_dd = find_element(p_window, "initial state"),
                  *saveas_button    = find_element(p_window, "ai save button"),
                  *err_lab          = find_element(p_window, "ai tool status");
                  
    // Pre set the active textbox
    p_instance->active_window->last = p_ai_path;
    add_state->button->y = 185;
    remove_state->button->y = 201;
    state_name_tinp->text_input->y = 184;
    saveas_button->button->y = 232;
    err_lab->label->y = 225;
    p_window->height = 254;

    err_lab->label->text[0] = '\0';

    p_ai_path = ((UIElement_t*)p_ai_path)->text_input;
    #ifdef _WIN64
        p_ai_path->width = 8 * ( sprintf(p_ai_path->text, "%s", path) + 1);
    #else
        p_ai_path->width = 8 * ( sprintf(p_ai_path->text, "%s", path) + 1);
    #endif

    // Load the file
    if ( ui_load_file(path, text, false) == 0 )
        return 0;

    // Parse the JSON text into a JSON value
    if ( parse_json_value(text, 0, &p_value) == 0 )
        return 0;

    // Parse the JSON value
    if ( p_value->type == JSONobject )
    {

        // Initialized data
        dict *p_dict = p_value->object;

        p_name          = dict_get(p_dict, "name");
        p_initial_state = dict_get(p_dict, "initial state");
        p_states        = dict_get(p_dict, "states");

        if ( ! ( p_name && p_initial_state && p_states ) )
            return 0;
    }

    // Populate the window
    if ( p_name->type == JSONstring )
    {

        // Initialized data
        size_t len = strlen(p_name->string);

        // Copy the string
        set_text_input_text(name->text_input, p_name->string);
    }

    // Populate the states table
    if ( p_states->type == JSONarray )
    {
        
        // Initialized data
        size_t len = 0;
        JSONValue_t **pp_state_names = 0;

        // Get the array contents
        {

            // Get the size of the array
            array_get(p_states->list, 0, &len);

            // Allocate memory for state names
            pp_state_names = calloc(len, sizeof(JSONValue_t *));

            // Error checking
            if ( pp_state_names == (void *) 0 )
                goto no_mem;

            // Get the name of each state
            array_get(p_states->list, pp_state_names, 0);
        }

        states->table->max_rows = len;

        for (size_t i = 0; i < len; i++)
            set_table_cell(states->table, 0, i, pp_state_names[i]->string);

        initial_state_dd->dropdown->longest_option = states->table->column_widths[0];
        initial_state_dd->dropdown->options_count = len;

        // Move elements down
        add_state->button->y += 12 * (len-1);
        remove_state->button->y += 12 * (len-1);
        state_name_tinp->text_input->y += 12 * (len-1);
        saveas_button->button->y += 12 * (len-1);
        err_lab->label->y += 12 * (len-1);
        p_window->height += 12 * (len-1);
        resize_window(p_window);
    }

    // Success
    return 1;

    no_mem:
        return 0;
}

void saveas_click (UIElement_t *p_element, ui_mouse_state_t mouse_state)
{
    
    // Initialized data
    UIInstance_t  *p_instance       = ui_get_active_instance();
    UIWindow_t    *p_window         = p_instance->active_window;
    UITextInput_t *p_ai_path        = find_element(p_window, "ai tool path");
    UIElement_t   *ai_name          = find_element(p_window, "ai name"),
                  *ai_initial_state = find_element(p_window, "initial state"),
                  *ai_states        = find_element(p_window, "states table"),
                  *err_lab          = find_element(p_window, "ai tool status");

    JSONValue_t  value         = { 0 },
                 name          = { 0 },
                 initial_state = { 0 },
                 states        = { 0 };
    dict        *value_object  = 0;
    array       *state_array   = 0;

    // Checks
    if ( strlen(ai_name->text_input->text) == 0 )
        strcpy(err_lab->label->text, "\210 Missing properties!");


    // Set the name
    name = (JSONValue_t)
    {
        .type = JSONstring,
        .string = _strdup(ai_name->text_input->text)
    };
    
    // Set the initial state
    initial_state = (JSONValue_t)
    {
        .type = JSONstring,
        .string = _strdup(ai_initial_state->dropdown->options[ai_initial_state->dropdown->index])
    };
    
    // Set the state array
    array_construct(&state_array, ai_states->table->max_rows);

    // Iterate over each state
    for (size_t i = 0; i < ai_states->table->max_rows; i++)
    {

        // Initialized data
        JSONValue_t *state_name = calloc(1, sizeof(JSONValue_t *));

        *state_name = (JSONValue_t)
        {
            .type   = JSONstring,
            .string = _strdup(ai_states->table->data[i])
        };

        array_add(state_array, state_name);
    }
    
    states = (JSONValue_t)
    {
        .type = JSONarray,
        .list = state_array
    };

    // Set the value
    dict_construct(&value_object, 3);
    dict_add(value_object, "name", &name);
    dict_add(value_object, "initial state", &initial_state);
    dict_add(value_object, "states", &states);

    value = (JSONValue_t)
    {
        .type = JSONobject,
        .object = value_object
    };

    FILE *f = fopen("C:/Users/j/Desktop/AI.json", "w");
    print_json_value(&value, f);
    fclose(f);
}

void click_elements_table (UIElement_t *p_element, ui_mouse_state_t mouse_state)
{
    UIInstance_t *p_instance   = ui_get_active_instance();
    UIWindow_t   *p_window     = p_instance->active_window;
    UIElement_t  *p_state_tinp = find_element(p_window, "add state name text input");
    UITable_t    *p_table = p_element;
    size_t off = p_table->last_y*p_table->max_columns+p_table->last_x;
    char *a = p_table->data[off];
    if ( a )
        set_text_input_text(p_state_tinp->text_input, a);

}

void remove_click (UIElement_t *p_element, ui_mouse_state_t mouse_state)
{
    UIInstance_t  *p_instance       = ui_get_active_instance();
    UIWindow_t    *p_window         = p_instance->active_window;
    UIElement_t   *state_name       = find_element(p_window, "add state name text input"),
                  *table            = find_element(p_window, "states table"),
                  *add_state        = find_element(p_window, "ai add state button"),
                  *remove_state     = find_element(p_window, "ai remove state button"),
                  *saveas_button    = find_element(p_window, "ai save button"),
                  *initial_state_dd = find_element(p_window, "initial state"),
                  *err_lab          = find_element(p_window, "ai tool status");
    UITextInput_t *p_text_input     = state_name->text_input;
    UITable_t     *p_table          = table->table;
    
    if ( p_table->max_rows == 1 )
    {
        set_table_cell(p_table, 0, 0, "");
        return;
    }
    
    for (size_t i = p_table->last_y; i < p_table->max_rows-1; i++)
    {
        set_table_cell(p_table, 0, i, "");
        set_table_cell(p_table, 0, i, get_table_cell(p_table, 0, i+1));
    }
    set_table_cell(p_table, 0, p_table->max_rows, "");

    p_table->max_rows--;
    initial_state_dd->dropdown->options_count--;
    initial_state_dd->dropdown->longest_option = p_table->column_widths[0];

    // Move elements down
    add_state->button->y -= 12;
    remove_state->button->y -= 12;
    state_name->text_input->y -= 12;
    saveas_button->button->y -= 12;
    p_window->height -= 12;
    err_lab->label->y -= 12;
    resize_window(p_window);
}

void add_click (UIElement_t *p_element, ui_mouse_state_t mouse_state)
{
    UIInstance_t  *p_instance       = ui_get_active_instance();
    UIWindow_t    *p_window         = p_instance->active_window;
    UIElement_t   *state_name       = find_element(p_window, "add state name text input"),
                  *table            = find_element(p_window, "states table"),
                  *add_state        = find_element(p_window, "ai add state button"),
                  *remove_state     = find_element(p_window, "ai remove state button"),
                  *saveas_button    = find_element(p_window, "ai save button"),
                  *initial_state_dd = find_element(p_window, "initial state"),
                  *err_lab          = find_element(p_window, "ai tool status");
    UITextInput_t *p_text_input     = state_name->text_input;
    UITable_t     *p_table          = table->table;
    
    char *z = _strdup(state_name->text_input->text);
    if ( get_table_cell(p_table, 0, 0) == 0 || strlen((char *)(get_table_cell(p_table, 0, 0))) == 0 )
    {
        set_table_cell(p_table, 0, 0, z);
        initial_state_dd->dropdown->longest_option = p_table->column_widths[0];
        return;
    }
    else
        set_table_cell(p_table, 0, p_table->max_rows, z);
    
    p_table->max_rows++;
    initial_state_dd->dropdown->options_count++;
    initial_state_dd->dropdown->longest_option = p_table->column_widths[0];

    state_name->text_input->text[0] = '\0';

    // Move elements down
    add_state->button->y += 12;
    remove_state->button->y += 12;
    state_name->text_input->y += 12;
    saveas_button->button->y += 12;
    err_lab->label->y += 12;
    p_window->height += 12;
    resize_window(p_window);
}

int main ( int argc, const char *argv[] )
{

    // Initialized data
    UIInstance_t  *p_instance       = 0;
    UIWindow_t    *p_window         = 0;
    UIElement_t   *p_saveas_button  = 0,
                  *states           = 0,
                  *rem_button       = 0,
                  *add_button       = 0,
                  *initial_state_dd = 0;
    UITextInput_t *p_shader_path    = 0;
    char          *g10_directory    = getenv("G10_SOURCE_PATH");
                  
    // Initialize the UI Library
    ui_init(&p_instance, "");

    // Load the window
    load_window(&p_window, "window.json");

    // Add the window to the instance
    ui_append_window(p_instance, p_window);

    // Find the save as button
    p_saveas_button = find_element(p_window, "ai save button");
    rem_button = find_element(p_window, "ai remove state button");
    add_button = find_element(p_window, "ai add state button");
    states = find_element(p_window, "states table");
    initial_state_dd = find_element(p_window, "initial state");

    states->table->max_rows = 1;

    // Add a callback to the save as
    add_click_callback_element(p_saveas_button, saveas_click);
    add_click_callback_element(states, click_elements_table);
    add_release_callback_element(rem_button, remove_click);
    add_release_callback_element(add_button, add_click);
    
    set_file_drop_operation(p_window, &drop_operation);
    set_table_cell(states->table, 0,0,"");
    initial_state_dd->dropdown->options = states->table->data;

    // Copy the G10 path
    {
        p_shader_path = find_element(p_window, "ai tool path");
    
        // Pre set the active textbox
        p_instance->active_window->last = p_shader_path;
    
        p_shader_path = ((UIElement_t*)p_shader_path)->text_input;
        #ifdef _WIN64
        p_shader_path->width = 8 * ( sprintf(p_shader_path->text, "%sG\\ais\\", (g10_directory) ? g10_directory : "") + 1);
        #else
        p_shader_path->width = 8 * ( sprintf(p_shader_path->text, "%sG/ais/", (g10_directory) ? g10_directory : "") + 1);
        #endif
    }
   
    // Start running
    p_instance->running = true;

    // UI looop
    while ( p_instance->running )
    {

        // Get input
        ui_process_input(p_instance);

        // Draw windows
        ui_draw(p_instance);
    }

    // Exit
    ui_exit(&p_instance);
    
    return EXIT_SUCCESS;
}