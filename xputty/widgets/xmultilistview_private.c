/*
 *                           0BSD 
 * 
 *                    BSD Zero Clause License
 * 
 *  Copyright (c) 2019 Hermann Meyer
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */


#include "xmultilistview_private.h"
#include "xtooltip.h"
#include <sys/stat.h>
#include <libgen.h>


void _draw_multi_listview(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (!w) return;
    set_pattern(w,&w->app->color_scheme->normal,&w->app->color_scheme->active,BACKGROUND_);
    cairo_paint (w->cr);
}

void _draw_multi_list(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    if (attrs.map_state != IsViewable) return;
    int width = attrs.width;
    int height = attrs.height;
    ViewMultiList_t *filelist = (ViewMultiList_t*)w->parent_struct;

    use_base_color_scheme(w, NORMAL_);
    cairo_rectangle(w->crb, 0, 0, width, height);
    cairo_fill (w->crb);
    cairo_set_font_size (w->crb, w->app->normal_font);
    cairo_text_extents_t extents;
    cairo_text_extents_t fextents;
    cairo_text_extents(w->crb,"Ay", &extents);
    double h = extents.height;

    int ax = max(1,width/filelist->item_width);
    int i = (int)max(0,adj_get_value(w->adj)*ax);
    int a = 0;
    int j = filelist->list_size<filelist->show_items+i ? 
      filelist->list_size : filelist->show_items+i;
    for(;i<j;i++) {
        int k = 0;
        for(;k<ax;k++) {
            if (filelist->check_dir) {
                struct stat sb;
                if (stat(filelist->list_names[i], &sb) == 0 && S_ISDIR(sb.st_mode)) {
                    cairo_scale(w->crb,0.2, 0.2);
                    cairo_set_source_surface (w->crb, filelist->folder,(filelist->icon_pos+(k*filelist->item_width))*5.0,((double)a+0.1)*filelist->item_height*5.0);
                    cairo_paint (w->crb);
                    cairo_scale(w->crb,5.0, 5.0);
                    //use_text_color_scheme(w, INSENSITIVE_);
                } else {
                    cairo_scale(w->crb,0.2, 0.2);
                    cairo_set_source_surface (w->crb, filelist->file,(filelist->icon_pos+(k*filelist->item_width))*5.0,((double)a+0.1)*filelist->item_height*5.0);
                    cairo_paint (w->crb);
                    cairo_scale(w->crb,5.0, 5.0);
                    //use_text_color_scheme(w,NORMAL_ );
                }
            }
            /** show label **/
            if(i == filelist->prelight_item && i == filelist->active_item)
                use_text_color_scheme(w, ACTIVE_);
            else if(i == filelist->prelight_item)
                use_text_color_scheme(w, PRELIGHT_);
            else if (i == filelist->active_item)
                use_text_color_scheme(w, SELECTED_);
            else
                use_text_color_scheme(w,INSENSITIVE_ );

            char label[124];
            memset(label, '\0', sizeof(char)*124);
            //strcpy(label,basename(filelist->list_names[i]));
            cairo_text_extents(w->crb, basename(filelist->list_names[i]), &extents);
            if (extents.width > filelist->item_width-10) {
                int len = ((filelist->item_width-5)/(extents.width/strlen(basename(filelist->list_names[i]))));
                strncpy(label,basename(filelist->list_names[i]), len-3);
                strcat(label,"...");
            } else {
                strcpy(label,basename(filelist->list_names[i]));
            }
            cairo_text_extents(w->crb, label, &fextents);
            cairo_move_to (w->crb, (k*filelist->item_width) + (filelist->item_width/2) - (fextents.width/2), (filelist->item_height*((double)a+1.0))+3.0 - (h*max(0.71,w->scale.ascale)));
            cairo_show_text(w->crb, label);
            cairo_new_path (w->crb);
            if (i == filelist->prelight_item && extents.width > (float)filelist->item_width-10) {
                tooltip_set_text(w,filelist->list_names[i]);
                w->flags |= HAS_TOOLTIP;
                show_tooltip(w);
            } else if (i == filelist->prelight_item && extents.width < (float)filelist->item_width-10){
                w->flags &= ~HAS_TOOLTIP;
                hide_tooltip(w);
            }
            if (k<ax-1 && i<j-1) i++;
            else break;
        }
        a++;
    }
}

void _update_view(void *w_) {
    Widget_t *w = (Widget_t*)w_;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    if (attrs.map_state != IsViewable) return;
    int width = attrs.width;
    //int height = attrs.height;
    ViewMultiList_t *filelist = (ViewMultiList_t*)w->parent_struct;
    cairo_push_group (w->crb);
    cairo_set_font_size (w->crb, w->app->normal_font);
    cairo_text_extents_t extents;
    cairo_text_extents_t fextents;
    cairo_text_extents(w->crb,"Ay", &extents);
    double h = extents.height;

    int ax = max(1,width/filelist->item_width);
    int i = (int)max(0,adj_get_value(w->adj)*ax);
    int a = 0;
    int j = filelist->list_size<filelist->show_items+i ? 
      filelist->list_size : filelist->show_items+i;
    for(;i<j;i++) {
        int k = 0;
        for(;k<ax;k++) {
            if (i != filelist->prelight_item && i != filelist->prev_prelight_item) {
                if (k<ax-1 && i<j-1) i++;
                continue;
            }
            use_base_color_scheme(w, NORMAL_);
            cairo_rectangle(w->crb, k*filelist->item_width, a*filelist->item_height, filelist->item_width, filelist->item_height);
            cairo_fill(w->crb);
            if (filelist->check_dir) {
                struct stat sb;
                if (stat(filelist->list_names[i], &sb) == 0 && S_ISDIR(sb.st_mode)) {
                    cairo_scale(w->crb,0.2, 0.2);
                    if (i == filelist->prelight_item) {
                        cairo_set_source_surface (w->crb, filelist->folder_select,(filelist->icon_pos+(k*filelist->item_width))*5.0,((double)a+0.1)*filelist->item_height*5.0);
                    } else {
                        cairo_set_source_surface (w->crb, filelist->folder,(filelist->icon_pos+(k*filelist->item_width))*5.0,((double)a+0.1)*filelist->item_height*5.0);
                    }
                    cairo_paint (w->crb);
                    cairo_scale(w->crb,5.0, 5.0);
                } else {
                    cairo_scale(w->crb,0.2, 0.2);
                    cairo_set_source_surface (w->crb, filelist->file,(filelist->icon_pos+(k*filelist->item_width))*5.0,((double)a+0.1)*filelist->item_height*5.0);
                    if (i == filelist->prelight_item)
                        cairo_set_operator (w->crb, CAIRO_OPERATOR_HARD_LIGHT);
                    cairo_paint (w->crb);
                    cairo_scale(w->crb,5.0, 5.0);
                    cairo_set_operator (w->crb, CAIRO_OPERATOR_OVER);
                }
            }
            /** show label **/
            if(i == filelist->prelight_item && i == filelist->active_item)
                use_text_color_scheme(w, ACTIVE_);
            else if(i == filelist->prelight_item)
                use_text_color_scheme(w, PRELIGHT_);
            else if (i == filelist->active_item)
                use_text_color_scheme(w, SELECTED_);
            else
                use_text_color_scheme(w,INSENSITIVE_ );

            char label[124];
            memset(label, '\0', sizeof(char)*124);
            //strcpy(label,basename(filelist->list_names[i]));
            cairo_text_extents(w->crb, basename(filelist->list_names[i]), &extents);
            if (extents.width > filelist->item_width-10) {
                int len = ((filelist->item_width-5)/(extents.width/strlen(basename(filelist->list_names[i]))));
                strncpy(label,basename(filelist->list_names[i]), len-3);
                strcat(label,"...");
            } else {
                strcpy(label,basename(filelist->list_names[i]));
            }

            cairo_text_extents(w->crb, label, &fextents);
            cairo_move_to (w->crb, (k*filelist->item_width) + (filelist->item_width/2) - (fextents.width/2), (filelist->item_height*((double)a+1.0))+3.0 - (h*max(0.71,w->scale.ascale)));
            cairo_show_text(w->crb, label);
            cairo_new_path (w->crb);
            if (i == filelist->prelight_item && extents.width > (float)filelist->item_width-10) {
                tooltip_set_text(w,filelist->list_names[i]);
                w->flags |= HAS_TOOLTIP;
                show_tooltip(w);
            } else if (i == filelist->prelight_item && extents.width < (float)filelist->item_width-10){
                w->flags &= ~HAS_TOOLTIP;
                hide_tooltip(w);
            }
            if (k<ax-1 && i<j-1) i++;
            else break;
        }
        a++;
    }
    cairo_pop_group_to_source (w->crb);
    cairo_paint (w->crb);
    cairo_push_group (w->cr);
    cairo_set_source_surface (w->cr, w->buffer,0,0);
    cairo_paint (w->cr);

    cairo_pop_group_to_source (w->cr);
    cairo_paint (w->cr);
}

void _multi_list_motion(void *w_, void* xmotion_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    ViewMultiList_t *filelist = (ViewMultiList_t*)w->parent_struct;
    XMotionEvent *xmotion = (XMotionEvent*)xmotion_;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    int width = attrs.width;
    int height = attrs.height;
    int h = floor(max(1,(height/filelist->item_height))) * filelist->item_height;
    int ax = max(1,width/filelist->item_width);
    int x_items = max(1,width/max(1,(width/filelist->item_width)));
    int _items = h/max(1,(height/filelist->item_height));
    int prelight_item = ((xmotion->y/_items)*ax) + (xmotion->x/x_items) + (int)max(0,adj_get_value(w->adj)*ax);
    if(prelight_item != filelist->prelight_item) {
        filelist->prev_prelight_item = filelist->prelight_item;
        filelist->prelight_item = prelight_item;
        hide_tooltip(w);
        _update_view(w);
    }
    //expose_widget(w);
}

void _multi_list_key_pressed(void *w_, void* xkey_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t* listview = (Widget_t*) w->parent;
    XKeyEvent *xkey = (XKeyEvent*)xkey_;
    ViewMultiList_t *filelist = (ViewMultiList_t*)w->parent_struct;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    int height = attrs.height;
    int _items = height/(height/filelist->item_height);
    filelist->prelight_item = xkey->y/_items  + (int)max(0,adj_get_value(w->adj));
    int nk = key_mapping(w->app->dpy, xkey);
    if (nk) {
        switch (nk) {
            case 3:
            case 4:
            case 5:
            case 6: filelist->prelight_item = xkey->y/_items  + (int)max(0,adj_get_value(w->adj));
            break;
            default:
            break;
        }
    }
    listview->func.key_press_callback(listview, xkey_, user_data);
}

void _multi_list_entry_released(void *w_, void* button_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    if (w->flags & HAS_POINTER) {
        ViewMultiList_t *filelist = (ViewMultiList_t*)w->parent_struct;
        XButtonEvent *xbutton = (XButtonEvent*)button_;
        XWindowAttributes attrs;
        XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
        int height = attrs.height;
        int _items = height/(height/filelist->item_height);
        int prelight_item = xbutton->y/_items  + (int)max(0,adj_get_value(w->adj));
        if (prelight_item > filelist->list_size-1) return;
        if(xbutton->button == Button4) {
            if(prelight_item != filelist->prelight_item) {
                filelist->prelight_item = prelight_item;
            }
        } else if (xbutton->button == Button5) {
            if(prelight_item != filelist->prelight_item) {
                filelist->prelight_item = prelight_item;
            }
        } else if(xbutton->button == Button1) {
            Widget_t* listview = (Widget_t*) w->parent;
            filelist->active_item = filelist->prelight_item;
            adj_set_value(listview->adj,filelist->active_item);
            listview->func.button_release_callback(listview,xbutton,user_data);
        } else if(xbutton->button == Button3) {
            Widget_t* listview = (Widget_t*) w->parent;
            listview->func.button_release_callback(listview,xbutton,user_data);
        }
    }
}

void _multi_list_entry_double_clicked(void *w_, void* button_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t* listview = (Widget_t*) w->parent;
    ViewMultiList_t *filelist = (ViewMultiList_t*)w->parent_struct;
    XButtonEvent *xbutton = (XButtonEvent*)button_;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    int width = attrs.width;
    int height = attrs.height;
    int h = floor(max(1,(height/filelist->item_height))) * filelist->item_height;
    int ax = max(1,width/filelist->item_width);
    int x_items = max(1,width/max(1,(width/filelist->item_width)));
    int _items = h/max(1,(height/filelist->item_height));
    int prelight_item = ((xbutton->y/_items)*ax) + (xbutton->x/x_items) + (int)max(0,adj_get_value(w->adj)*ax);
    if (prelight_item > filelist->list_size-1) return;
    listview->func.double_click_callback(listview,button_,NULL);
}

void _leave_multi_list(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    ViewMultiList_t *filelist = (ViewMultiList_t*)w->parent_struct;
    filelist->prelight_item = -1;
    expose_widget(w);
}

void _reconfigure_multi_listview_viewport(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    float st = adj_get_state(w->adj);
    Widget_t* listview = (Widget_t*) w->parent;
    ViewMultiList_t *filelist = (ViewMultiList_t*)w->parent_struct;
    XWindowAttributes attrs;
    XGetWindowAttributes(listview->app->dpy, (Window)listview->widget, &attrs);
    int width = attrs.width;
    int height = attrs.height;
    int ax = max(1,width/filelist->item_width);
    filelist->show_items = (height/filelist->item_height) * ax;
    filelist->icon_pos = (filelist->item_width/2) - 120/5;
    w->adj->max_value = ((filelist->list_size-filelist->show_items)/ax)+1.0;
    filelist->slider->adj->max_value = ((filelist->list_size-filelist->show_items)/ax)+1.0;
    adj_set_scale(filelist->slider->adj, ((float)filelist->list_size/(float)filelist->show_items)/filelist->item_height);
    adj_set_state(w->adj,st);
}

void _set_multi_listview_viewpoint(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    ViewMultiList_t *filelist = (ViewMultiList_t*)w->parent_struct;
    adj_set_state(filelist->slider->adj,adj_get_state(w->adj));
    expose_widget(w);
}

void _draw_multi_listviewslider(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t* listview = (Widget_t*) w->parent;
    Widget_t* view_port = (Widget_t*)w->parent_struct;
    ViewMultiList_t *filelist = (ViewMultiList_t*)view_port->parent_struct;
    int v = (int)w->adj->max_value;
    if (!v) return;
    XWindowAttributes attrs;
    XGetWindowAttributes(w->app->dpy, (Window)w->widget, &attrs);
    if (attrs.map_state != IsViewable) return;
    int width = attrs.width;
    int height = attrs.height;
    XWindowAttributes vattrs;
    XGetWindowAttributes(w->app->dpy, (Window)listview->widget, &vattrs);
    int ax = max(1,vattrs.width/filelist->item_width);
    int show_items = (height/filelist->item_height) * ax;
    float slidersize = 1.0;
    if (filelist->list_size > show_items)
        slidersize = (float)((float)show_items/(float)filelist->list_size);
    float sliderstate = adj_get_state(w->adj);
    use_bg_color_scheme(w, get_color_state(w));
    cairo_rectangle(w->crb, 0,0,width,height);
    cairo_fill_preserve(w->crb);
    use_shadow_color_scheme(w, NORMAL_);
    cairo_fill(w->crb);
    use_bg_color_scheme(w, NORMAL_);
    cairo_rectangle(w->crb, 0,((float)height-
        ((float)height*slidersize))*sliderstate,width,((float)height*slidersize));
    cairo_fill(w->crb);
}

void _set_multi_listviewport(void *w_, void* user_data) {
    Widget_t *w = (Widget_t*)w_;
    Widget_t *viewport = (Widget_t*)w->parent_struct;
    adj_set_state(viewport->adj, adj_get_state(w->adj));
    expose_widget(w);
}
