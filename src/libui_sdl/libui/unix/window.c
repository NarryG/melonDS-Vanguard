// 11 june 2015
#include "uipriv_unix.h"

struct uiWindow {
	uiUnixControl c;

	GtkWidget *widget;
	GtkContainer *container;
	GtkWindow *window;

	GtkWidget *vboxWidget;
	GtkContainer *vboxContainer;
	GtkBox *vbox;

	GtkWidget *childHolderWidget;
	GtkContainer *childHolderContainer;

	GtkWidget *menubar;

	uiControl *child;
	int margined;
	
	int width, height;

	int (*onClosing)(uiWindow *, void *);
	void *onClosingData;
	void (*onContentSizeChanged)(uiWindow *, void *);
	void *onContentSizeChangedData;
	void (*onDropFile)(uiWindow *, char *, void *);
	void *onDropFileData;
	void (*onGetFocus)(uiWindow *, void *);
	void *onGetFocusData;
	void (*onLoseFocus)(uiWindow *, void *);
	void *onLoseFocusData;
	
	gboolean fullscreen;
};

static gboolean onClosing(GtkWidget *win, GdkEvent *e, gpointer data)
{
	uiWindow *w = uiWindow(data);

	// manually destroy the window ourselves; don't let the delete-event handler do it
	if ((*(w->onClosing))(w, w->onClosingData))
		uiControlDestroy(uiControl(w));
	// don't continue to the default delete-event handler; we destroyed the window by now
	return TRUE;
}

static void onSizeAllocate(GtkWidget *widget, GdkRectangle *allocation, gpointer data)
{
	uiWindow *w = uiWindow(data);

	// TODO deal with spurious size-allocates
	(*(w->onContentSizeChanged))(w, w->onContentSizeChangedData);
}

static gboolean onGetFocus(GtkWidget *win, GdkEvent *e, gpointer data)
{
    uiWindow *w = uiWindow(data);
    if (w->onGetFocus)
        w->onGetFocus(w, w->onGetFocusData);
}

static gboolean onLoseFocus(GtkWidget *win, GdkEvent *e, gpointer data)
{
    uiWindow *w = uiWindow(data);
    if (w->onLoseFocus)
        w->onLoseFocus(w, w->onLoseFocusData);
}

static int defaultOnClosing(uiWindow *w, void *data)
{
	return 0;
}

static void defaultOnPositionContentSizeChanged(uiWindow *w, void *data)
{
	// do nothing
}

static void uiWindowDestroy(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	// first hide ourselves
	gtk_widget_hide(w->widget);
	// now destroy the child
	if (w->child != NULL) {
		uiControlSetParent(w->child, NULL);
		uiUnixControlSetContainer(uiUnixControl(w->child), w->childHolderContainer, TRUE);
		uiControlDestroy(w->child);
	}
	// now destroy the menus, if any
	if (w->menubar != NULL)
		freeMenubar(w->menubar);
	gtk_widget_destroy(w->childHolderWidget);
	gtk_widget_destroy(w->vboxWidget);
	// and finally free ourselves
	// use gtk_widget_destroy() instead of g_object_unref() because GTK+ has internal references (see #165)
	gtk_widget_destroy(w->widget);
	uiFreeControl(uiControl(w));
}

void uiWindowSetPosition(uiWindow *w, int x, int y)
{
	if (!w) return;

	gtk_window_move(w->window, x, y);
}

void uiWindowPosition(uiWindow *w, int *x, int *y)
{
	if (!w) return;

	int xx, yy;
	gtk_window_get_position(w->window, &xx, &yy);
	if (x) *x = xx;
	if (y) *y = yy;
}

uiUnixControlDefaultHandle(uiWindow)

uiControl *uiWindowParent(uiControl *c)
{
	return NULL;
}

void uiWindowSetParent(uiControl *c, uiControl *parent)
{
	uiUserBugCannotSetParentOnToplevel("uiWindow");
}

static int uiWindowToplevel(uiControl *c)
{
	return 1;
}

uiUnixControlDefaultVisible(uiWindow)

static void uiWindowShow(uiControl *c)
{
	uiWindow *w = uiWindow(c);

	// don't use gtk_widget_show_all() as that will show all children, regardless of user settings
	// don't use gtk_widget_show(); that doesn't bring to front or give keyboard focus
	// (gtk_window_present() does call gtk_widget_show() though)
	gtk_window_present(w->window);
	
	// set the size properly
	int width = w->width;
	int height = w->height;
	if (w->menubar)
	{
	    GtkRequisition min, nat;
	    int menuheight;
	    gtk_widget_get_preferred_size(w->menubar, &min, &nat);
	    menuheight = min.height;
	    if (nat.height > menuheight) menuheight = nat.height;
	    height += menuheight;
	}
	gtk_window_resize(w->window, width, height);
}

static void uiWindowSetFocus(uiControl* c)
{
    gtk_window_present(GTK_WINDOW(uiWindow(c)->widget));
}

uiUnixControlDefaultHide(uiWindow)
uiUnixControlDefaultEnabled(uiWindow)
uiUnixControlDefaultEnable(uiWindow)
uiUnixControlDefaultDisable(uiWindow)
//uiUnixControlDefaultSetFocus(uiWindow)
uiUnixControlDefaultSetMinSize(uiWindow)
// TODO?
uiUnixControlDefaultSetContainer(uiWindow)

char *uiWindowTitle(uiWindow *w)
{
	return uiUnixStrdupText(gtk_window_get_title(w->window));
}

void uiWindowSetTitle(uiWindow *w, const char *title)
{
	gtk_window_set_title(w->window, title);
}

void uiWindowContentSize(uiWindow *w, int *width, int *height)
{
	GtkAllocation allocation;

	gtk_widget_get_allocation(w->childHolderWidget, &allocation);
	*width = allocation.width;
	*height = allocation.height;
}

void uiWindowSetContentSize(uiWindow *w, int width, int height)
{
	GtkAllocation childAlloc;
	gint winWidth, winHeight;

	// we need to resize the child holder widget to the given size
	// we can't resize that without running the event loop
	// but we can do gtk_window_set_size()
	// so how do we deal with the differences in sizes?
	// simple arithmetic, of course!

	// from what I can tell, the return from gtk_widget_get_allocation(w->window) and gtk_window_get_size(w->window) will be the same
	// this is not affected by Wayland and not affected by GTK+ builtin CSD
	// so we can safely juse use them to get the real window size!
	// since we're using gtk_window_resize(), use the latter
	gtk_window_get_size(w->window, &winWidth, &winHeight);

	// now get the child holder widget's current allocation
	gtk_widget_get_allocation(w->childHolderWidget, &childAlloc);
	// and punch that out of the window size
	winWidth -= childAlloc.width;
	winHeight -= childAlloc.height;

	// now we just need to add the new size back in
	winWidth += width;
	winHeight += height;
	// and set it
	// this will not move the window in my tests, so we're good
	gtk_window_resize(w->window, winWidth, winHeight);
}

int uiWindowMinimized(uiWindow *w)
{
    // TODO!!
    return 0;
}

void uiWindowSetMinimized(uiWindow *w, int minimized)
{
    if (minimized)
        gtk_window_iconify(w->window);
    else
        gtk_window_deiconify(w->window);
}

int uiWindowMaximized(uiWindow *w)
{
    return gtk_window_is_maximized(w->window);
}

void uiWindowSetMaximized(uiWindow *w, int maximized)
{
    if (maximized)
        gtk_window_maximize(w->window);
    else
        gtk_window_unmaximize(w->window);
}


int uiWindowFullscreen(uiWindow *w)
{
	return w->fullscreen;
}

// TODO use window-state-event to track
// TODO does this send an extra size changed?
// TODO what behavior do we want?
void uiWindowSetFullscreen(uiWindow *w, int fullscreen)
{
	w->fullscreen = fullscreen;
	if (w->fullscreen)
		gtk_window_fullscreen(w->window);
	else
		gtk_window_unfullscreen(w->window);
}

void uiWindowOnContentSizeChanged(uiWindow *w, void (*f)(uiWindow *, void *), void *data)
{
	w->onContentSizeChanged = f;
	w->onContentSizeChangedData = data;
}

void uiWindowOnClosing(uiWindow *w, int (*f)(uiWindow *, void *), void *data)
{
	w->onClosing = f;
	w->onClosingData = data;
}

void uiWindowOnDropFile(uiWindow *w, void (*f)(uiWindow *, char *, void *), void *data)
{
	w->onDropFile = f;
	w->onDropFileData = data;
}

void uiWindowOnGetFocus(uiWindow *w, void (*f)(uiWindow *, void *), void *data)
{
	w->onGetFocus = f;
	w->onGetFocusData = data;
}

void uiWindowOnLoseFocus(uiWindow *w, void (*f)(uiWindow *, void *), void *data)
{
	w->onLoseFocus = f;
	w->onLoseFocusData = data;
}

int uiWindowBorderless(uiWindow *w)
{
	return gtk_window_get_decorated(w->window) == FALSE;
}

void uiWindowSetBorderless(uiWindow *w, int borderless)
{
	gtk_window_set_decorated(w->window, borderless == 0);
}

// TODO save and restore expands and aligns
void uiWindowSetChild(uiWindow *w, uiControl *child)
{
	if (w->child != NULL) {
		uiControlSetParent(w->child, NULL);
		uiUnixControlSetContainer(uiUnixControl(w->child), w->childHolderContainer, TRUE);
	}
	w->child = child;
	if (w->child != NULL) {
		uiControlSetParent(w->child, uiControl(w));
		uiUnixControlSetContainer(uiUnixControl(w->child), w->childHolderContainer, FALSE);
	}
}

int uiWindowMargined(uiWindow *w)
{
	return w->margined;
}

void uiWindowSetMargined(uiWindow *w, int margined)
{
	w->margined = margined;
	setMargined(w->childHolderContainer, w->margined);
}

static void onDragDataReceived(GtkWidget* widget, GdkDragContext* ctx, gint x, gint y, GtkSelectionData* data, guint info, guint time, gpointer userdata)
{
    uiWindow* w = (uiWindow*)userdata;
    
    if (gtk_selection_data_get_length(data) > 0 && gtk_selection_data_get_format(data) == 8)
    {
        gchar** files = gtk_selection_data_get_uris(data);
        if (files != NULL && files[0] != NULL)
        {
            // TODO: multi file support?
            
            gboolean success = FALSE;
            
            gchar* file = g_filename_from_uri(files[0], NULL, NULL);
            if (file)
            {
                if (w->onDropFile)
                    w->onDropFile(w, file, w->onDropFileData);
                    
                success = TRUE;
                g_free(file);
            }
            
            
            g_strfreev(files);
            gtk_drag_finish(ctx, success, FALSE, time);
            return;
        }
        
        if (files != NULL) g_strfreev(files);
        gtk_drag_finish(ctx, FALSE, FALSE, time);
    }
}

void uiWindowSetDropTarget(uiWindow* w, int drop)
{
    if (!drop)
    {
        gtk_drag_dest_unset(w->widget);
        return;
    }

    GtkTargetEntry entry;
    entry.target = "text/uri-list";
    entry.flags = GTK_TARGET_OTHER_APP;
    entry.info = 1;
    
    // CHECKME: action copy?
    gtk_drag_dest_set(w->widget, GTK_DEST_DEFAULT_ALL, &entry, 1, GDK_ACTION_COPY|GDK_ACTION_MOVE);
    
    g_signal_connect(w->widget, "drag-data-received", G_CALLBACK(onDragDataReceived), w);
}

uiWindow *uiNewWindow(const char *title, int width, int height, int maximized, int hasMenubar, int resizable)
{
	uiWindow *w;
	
	if (!resizable) maximized = 0;

	uiUnixNewControl(uiWindow, w);

	w->widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	w->container = GTK_CONTAINER(w->widget);
	w->window = GTK_WINDOW(w->widget);

	gtk_window_set_title(w->window, title);
	gtk_window_resize(w->window, width, height);

	w->vboxWidget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	w->vboxContainer = GTK_CONTAINER(w->vboxWidget);
	w->vbox = GTK_BOX(w->vboxWidget);

	// set the vbox as the GtkWindow child
	gtk_container_add(w->container, w->vboxWidget);

	if (hasMenubar) {
		w->menubar = makeMenubar(uiWindow(w));
		gtk_container_add(w->vboxContainer, w->menubar);
	}
	else
	    w->menubar = NULL;

	w->childHolderWidget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	w->childHolderContainer = GTK_CONTAINER(w->childHolderWidget);
	gtk_widget_set_hexpand(w->childHolderWidget, TRUE);
	gtk_widget_set_halign(w->childHolderWidget, GTK_ALIGN_FILL);
	gtk_widget_set_vexpand(w->childHolderWidget, TRUE);
	gtk_widget_set_valign(w->childHolderWidget, GTK_ALIGN_FILL);
	gtk_box_set_homogeneous(GTK_BOX(w->childHolderWidget), TRUE);
	gtk_container_add(w->vboxContainer, w->childHolderWidget);

	// show everything in the vbox, but not the GtkWindow itself
	gtk_widget_show_all(w->vboxWidget);

	// and connect our events
	g_signal_connect(w->widget, "delete-event", G_CALLBACK(onClosing), w);
	g_signal_connect(w->childHolderWidget, "size-allocate", G_CALLBACK(onSizeAllocate), w);
	g_signal_connect(w->widget, "focus-in-event", G_CALLBACK(onGetFocus), w);
	g_signal_connect(w->widget, "focus-out-event", G_CALLBACK(onLoseFocus), w);
	
	uiWindowOnClosing(w, defaultOnClosing, NULL);
	uiWindowOnContentSizeChanged(w, defaultOnPositionContentSizeChanged, NULL);
	
	uiWindowOnDropFile(w, NULL, NULL);
	uiWindowOnGetFocus(w, NULL, NULL);
	uiWindowOnLoseFocus(w, NULL, NULL);

	// normally it's SetParent() that does this, but we can't call SetParent() on a uiWindow
	// TODO we really need to clean this up, especially since see uiWindowDestroy() above
	g_object_ref(w->widget);
	
	gtk_window_set_resizable(w->window, resizable?TRUE:FALSE);
	
	w->width = width;
	w->height = height;
	
	if (maximized)
	    gtk_window_maximize(w->window);
	else
	    gtk_window_set_position(w->window, GTK_WIN_POS_CENTER);

	return w;
}

