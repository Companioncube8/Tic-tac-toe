#include <gtk/gtk.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <time.h> 
#include <netdb.h> 
#include <pthread.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
    
#define MAXDATASIZE 100 
#define nlen 21
#define TEXT 1
#define JOIN 2
#define NEW 3
#define STEP 4
#define PING 8
#define RECONNECT 10
int num_error;
char *PORT[] = {"1400", "1401", "1402"};

int num_p;
int player;
int play_gr[3][3];

pthread_t thread;
GtkWidget *contain1, *contain11;
GtkWidget *contain22;
GtkWidget *pl_gr1[3][3];
GtkWidget *pl_gr2[3][3];
GtkWidget *pl_gr_px[3][3];
GtkWidget *pl_gr_str[3];
GtkWidget *window;
GtkWidget  *view;
gchar matrx[] = "000102101112202122";
int sockfd;
int numbytes; 
int game_number;
int pic;
char buf[MAXDATASIZE];
char cmd[3];
int numberpl;
GtkWidget *entry;
GtkWidget *contain;
GtkWidget *contain2;
GtkWidget *button;
GtkTextBuffer *buffer;
void callback_enter_name  (GtkWidget * widget, gpointer data);

struct msg
{
	int type;
 	char buf[100];
	int size;
	int s;
};
	
void callback_pl_b (GtkWidget * widget, gpointer data);
GtkWidget *xpm_label_box(GtkWidget *parent, gchar *xpm_filename);
void callback_jg (GtkWidget * widget, gpointer data);
void pl_cr();
void callback_pl_b1 (GtkWidget * widget, gpointer data);
void crt_main_menu();
int connect_to_server();
void* ping (void *args);

GtkWidget *xpm_label_box(
	GtkWidget *parent,
	gchar *xpm_filename)
{
	GtkWidget *box1;
	GtkWidget *pixmapwid;
	GdkPixmap *pixmap;
	GdkBitmap *mask;
	GtkStyle *style;

	box1 = gtk_hbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (box1), 2);


	style = gtk_widget_get_style(parent);

	pixmap = gdk_pixmap_create_from_xpm(
		parent->window, &mask,
		&style->bg[GTK_STATE_NORMAL],
		xpm_filename);
	pixmapwid = gtk_pixmap_new (pixmap, mask);
	gtk_box_pack_start (GTK_BOX (box1),
		pixmapwid, FALSE, FALSE, 3);

	gtk_widget_show(pixmapwid);

	return(box1);
}

gint delete_event (GtkWidget * widget, GdkEvent * event, gpointer data)
{
	gtk_main_quit ();
	return (FALSE);
}

void change_butt(int i, int j, int num)
{
	GtkWidget *button;
	
	
	gtk_widget_destroy (pl_gr2[i][j]);
		
	pl_gr2[i][j] = gtk_hbox_new(0, 0);
	pl_gr_px[i][j] = gtk_hbox_new(0, 0);
	button = gtk_button_new();
			
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	GTK_SIGNAL_FUNC (callback_pl_b1), (gpointer) (gpointer) (matrx + ((3*i + j) * 2)));
				
	
	if (num == 1) {
		if (pic == 0){
			pl_gr_px[i][j] = xpm_label_box(window, "x.png");
			play_gr[j][i] = 2;
		}
		else{
			pl_gr_px[i][j] = xpm_label_box(window, "o.png");
			play_gr[j][i] = 1;
		}
	} else {
		if (pic == 0){
			pl_gr_px[i][j] = xpm_label_box(window, "o.png");
			play_gr[j][i] = 1;
		}
		else{
			pl_gr_px[i][j] = xpm_label_box(window, "x.png");
			play_gr[j][i] = 2;
		}
	}
	gtk_widget_show(pl_gr_px[i][j]);
	gtk_container_add (GTK_CONTAINER (button), pl_gr_px[i][j]);
				
	gtk_box_pack_start(GTK_BOX(pl_gr2[i][j]), button, TRUE, 10, 0);
	gtk_widget_show (button);
	gtk_box_pack_start(GTK_BOX(pl_gr1[i][j]), pl_gr2[i][j], TRUE, 10, 0);
	gtk_widget_show(pl_gr2[i][j]);
}


gboolean funct123(gpointer data)
{
	int numbytes = recv(sockfd, &game_number, sizeof(int), 0);
	if (numbytes == -1){ 
		return TRUE;
	} else {
		numberpl = 1;
		player = 1;
		pic = 1;
		gtk_widget_destroy (contain);
		pl_cr();
		return FALSE;
	}
}

void crt_end_menu(int cmd)
{
	GtkWidget *label;
	
	if (cmd == 0) {
		label = gtk_label_new("Вы проиграли!");
	}
	else {
		if (cmd == 1){
			label = gtk_label_new("Вы выиграли!");
		}
		else{ 
			if (cmd == 2){
				label = gtk_label_new("Ничья!");
			}
			else {
				if (cmd == -3){
					label = gtk_label_new("Сервера не работают");
				}
				else {
					if (cmd == -2){
						label = gtk_label_new("Ваш противник вышел");
					}
				}
			}
		}
	}
		
	gtk_widget_destroy (GTK_WIDGET(contain));
 	
	contain = gtk_vbox_new(0, 0);
	contain2 = gtk_vbox_new(0, 0);
	contain1 = gtk_vbox_new(0, 0);
	gtk_box_pack_start(GTK_BOX(contain), label, TRUE, 10, 0);
	gtk_widget_show(label);
	
	button = gtk_button_new_with_label ("Quit");
	gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (delete_event), GTK_OBJECT (window));
	gtk_box_pack_start(GTK_BOX(contain2), button, TRUE, 10, 10);
	gtk_widget_show (button);
	
	gtk_box_pack_start(GTK_BOX(contain1), contain, TRUE, 10, 10);
	gtk_box_pack_start(GTK_BOX(contain1), contain2, TRUE, 10, 10);
	gtk_widget_show (contain);
	gtk_widget_show (contain2);

	gtk_container_add (GTK_CONTAINER(window), contain1);
	gtk_widget_show(contain1);
	gtk_widget_show (window);
}

 
gboolean pl_b(gpointer data)
{
	int i, j;
	struct msg *message = malloc(sizeof(struct msg));
	int numbytes = recv(sockfd, message, sizeof(struct msg), MSG_DONTWAIT);
	if (errno == 9){
		perror("recv");
		crt_end_menu(-3);
		return FALSE;
	} 
	if (numberpl == 2){
		if (numbytes == -1) {
				return TRUE;
		} else {
			if (message->type == 4){
				i = message->s / 10;
				j = message->s % 10;
				if (i < 3 && i >= 0 && j < 3 && j >= 0){
					change_butt(i, j, 2);
					if ((numbytes = recv(sockfd, message, sizeof(struct msg), 0)) == -1) { 
						if (errno == 9){
							perror("recv");
							crt_end_menu(-3);
							return FALSE;
						}  
					}
					if (message->s == 1 || message->s == 0|| message->s == 2)
					{
						crt_end_menu(message->s);
					}
					numberpl = 1;
					return TRUE;
				}
			}
			if (message->type == 1){
				gtk_text_buffer_insert_at_cursor (buffer, "op: ", -1);
				gtk_text_buffer_insert_at_cursor (buffer, message->buf, -1);
				gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), buffer);
				return TRUE;	
			}
			if (message->type == 11){
				 crt_end_menu(-2);
			}
		}
	} else{
		if (numbytes == -1) {
			return TRUE;
		} else {
			if (message->type == 4){
				if (message->s == 1 || message->s == 0|| message->s == 2) 
				{
					crt_end_menu(message->s);
				}
				numberpl = 2;
				return TRUE;
			}
			if (message->type == 1){
				gtk_text_buffer_insert_at_cursor (buffer, "op: ", -1);
				gtk_text_buffer_insert_at_cursor (buffer, message->buf, -1);
				gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), buffer);
				return TRUE;	
			}
			if (message->type == 11){
				 crt_end_menu(-2);
			}
		}
	}
}
 
 void callback_pl_b (GtkWidget * widget, gpointer data) {

	gchar *name1;
	name1 = (gchar *) data;
	if (numberpl == 1) {
		int i = name1[0] - '0';
		int j = name1[1] - '0';
		struct msg *message = malloc(sizeof(struct msg));
		message->type = STEP;
		message->s = i * 10 + j; 
		message->size = 0;
		memset(message->buf, '\0', 100);
		if (send(sockfd, message, sizeof(struct msg), 0) == -1) {
			perror("send");
			sleep(10);
			send(sockfd, message, sizeof(struct msg), 0);
		}
		change_butt(i, j, 1);
	}
}	
 

void callback_pl_b1 (GtkWidget * widget, gpointer data) {

}


void callback_send_mgs(GtkButton* widget, gpointer data)
{
	struct msg *message = malloc(sizeof(struct msg));
    GtkEntry* ent = (GtkEntry*)data;
    gtk_entry_get_text_length (ent);
    const gchar* textFromEntry = gtk_entry_get_text (GTK_ENTRY (ent));
    strcpy(message->buf, textFromEntry);
    int l = strlen(message->buf);
    message->buf[l] = '\n';
    message->buf[l + 1] = '\0';
    gtk_text_buffer_insert_at_cursor (buffer, "you: ", -1);
	gtk_text_buffer_insert_at_cursor (buffer, message->buf, -1);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), buffer);
	gtk_entry_set_text (GTK_ENTRY(entry), "");
    message->type = 1;
    message->size = 0;
 	message->s = 0; 
	if (send(sockfd, message, sizeof(struct msg), 0) == -1) {
		perror("send");
	}
}

int MyAlarm ()
{
	int i, j;
	struct msg *message = malloc(sizeof(struct msg));
 	message->type = PING;
 	memset(message->buf, '\0', 100); 
 	message->size = 0;
 	message->s = 0; 
	if (send (sockfd, message, sizeof(struct msg), 0) == -1){
		perror("send");
		if (connect_to_server() == 0){
				message->type = RECONNECT;
				message->s = game_number;
				message->size = player;
				send (sockfd, message, sizeof(struct msg), 0);
				recv (sockfd, message, sizeof(struct msg), 0);
				if (strlen(message->buf) <= 9){
					char ch;
					int temp;
					for (i = 0; i < 3; i++)
						for (j = 0; j < 3; j++){
							ch = message->buf[(i * 3) + j];
							temp = atoi(&ch);
							play_gr[i][j] = temp;
					}
				}
				gtk_widget_destroy (GTK_WIDGET(contain));
				pl_cr();
				sleep(4);
				return FALSE;
		}
	}
	return TRUE;
}

/*void* ping (void *args)
{
	struct msg *message = malloc(sizeof(struct msg));
	time_t t;
	t = time(NULL);
	
	while(1){
		if(time(NULL) - t > 3){
			if (MyAlarm() == -1){
				if (connect_to_server() == 0){
					message->type = RECONNECT;
					message->s = game_number;
					message->size = player;
					send (sockfd, message, sizeof(struct msg), 0);
					pl_cr();
					sleep(4);
				} else{
					return 0;
				}
			}
			t = time(NULL);
		}
	}
}*/

void pl_cr()
{
	int i, j;
	GtkWidget *button;
	GtkWidget *table;
	
	contain11 = gtk_hbox_new(0, 0);
	contain = gtk_hbox_new(0, 0);
	table = gtk_table_new (5, 8, TRUE);

	
    int status;
    
   // status = pthread_create(&thread, NULL, ping, NULL);
	
	for(i = 0; i < 3; i++) {
		pl_gr_str[i]= gtk_vbox_new(0, 0);
		gtk_box_pack_start(GTK_BOX(contain11), pl_gr_str[i], TRUE, 10, 0);
		gtk_widget_show(pl_gr_str[i]);
	}
	
	for(i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			pl_gr1[i][j] = gtk_hbox_new(0, 0);
			gtk_box_pack_start(GTK_BOX(pl_gr_str[i]), pl_gr1[i][j], TRUE, 10, 0);
			gtk_widget_show(pl_gr1[i][j]);
		}
	}
	for(i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			pl_gr2[i][j] = gtk_hbox_new(0, 0);

			button = gtk_button_new();
			gtk_signal_connect (GTK_OBJECT (button), "clicked",
				GTK_SIGNAL_FUNC (callback_pl_b), (gpointer) (matrx + ((3*i + j) * 2)));
			if (play_gr[j][i] == 0)
				pl_gr_px[i][j] = xpm_label_box(window, "zero.png");
			if (play_gr[j][i] == 2)
				pl_gr_px[i][j] = xpm_label_box(window, "x.png");
			if (play_gr[j][i] == 1)
				pl_gr_px[i][j] = xpm_label_box(window, "o.png");
			gtk_widget_show(pl_gr_px[i][j]);
			gtk_container_add (GTK_CONTAINER (button), pl_gr_px[i][j]);
			
			gtk_box_pack_start(GTK_BOX(pl_gr2[i][j]), button, TRUE, 10, 0);
			gtk_widget_show (button);
			gtk_box_pack_start(GTK_BOX(pl_gr1[i][j]), pl_gr2[i][j], TRUE, 10, 0);
			gtk_widget_show(pl_gr2[i][j]);
		}
	}
	gtk_table_attach_defaults (GTK_TABLE (table), contain11, 0, 3, 0, 3);
	gtk_widget_show(contain11);
	gtk_widget_show(contain);
	
	entry = gtk_entry_new();
	
	gtk_table_attach_defaults (GTK_TABLE (table), entry, 0, 5, 4, 5);
	gtk_widget_show(entry);
	
	button = gtk_button_new_with_label ("Send");
	gtk_signal_connect (GTK_OBJECT (button), "clicked", G_CALLBACK (callback_send_mgs), (gpointer) entry);
	gtk_table_attach_defaults (GTK_TABLE (table), button, 6, 8, 4, 5);
	gtk_widget_show(button);
	
	view = gtk_text_view_new ();
	GtkWidget *scroll;
	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE); 
	gtk_container_add(GTK_CONTAINER(scroll), view);
	gtk_table_attach_defaults(GTK_TABLE (table), scroll, 4, 8, 0, 3);
	buffer  = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	gtk_text_buffer_set_text(buffer, "Wellcome to chat\n", -1);

	gtk_container_add (GTK_CONTAINER (contain), table);
	gtk_container_add (GTK_CONTAINER (window), contain);
	gtk_widget_show(table);
	gtk_widget_show(view);
	gtk_widget_show(scroll);
	g_timeout_add_seconds(1, pl_b, NULL);
	g_timeout_add_seconds(3, MyAlarm, NULL);
}

void print_error(GtkWidget * widget, gpointer data)
{
	gtk_widget_destroy (GTK_WIDGET(data));
	GtkWidget *label;
	label = gtk_label_new("Error: game name too big or too small");
	gtk_box_pack_start(GTK_BOX(contain11), label, TRUE, 10, 0);
	gtk_widget_show(label);
	button = (GtkButton *)gtk_button_new_with_label ("tried again");
	
	gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (callback_enter_name), contain11);
	gtk_box_pack_start(GTK_BOX(contain11), button, TRUE, 10, 0);
	gtk_widget_show (button);
	gtk_box_pack_start(GTK_BOX(contain1), contain11, TRUE, 10, 10);
	gtk_widget_show(contain11);
}

void callback_connect (GtkWidget * widget, gpointer data)
{
	struct msg *message = malloc(sizeof(struct msg));
	GtkWidget *label;
 	contain11 = gtk_vbox_new(0, 0);
 	int cmd = 0, len;
 	if (connect_to_server() != -1){
 	
		message->type = NEW;
		message->size = cmd;
		message->s = 0; 
		strcpy(message->buf, gtk_entry_get_text(GTK_ENTRY(entry)));
		if ((numbytes = send(sockfd, message, sizeof(struct msg), 0)) == -1) { 
			perror("send"); 
			crt_end_menu(-3);
		}
		recv(sockfd, message, sizeof(struct msg), 0);
		if (message->type == -1)
		{
			print_error(widget, data);
			return 0;
		}
		gtk_widget_destroy (GTK_WIDGET(data));
		
		label = gtk_label_new("Opponent waiting...");
		gtk_box_pack_start(GTK_BOX(contain11), label, TRUE, 10, 0);
		gtk_widget_show(label);
		
		gtk_box_pack_start(GTK_BOX(contain1), contain11, TRUE, 10, 10);
		gtk_widget_show(contain11);
		
		g_timeout_add_seconds(1, funct123, NULL);
	} else{
		crt_end_menu(-3);
	}
}

void callback_enter_name (GtkWidget * widget, gpointer data)
{
 	gtk_widget_destroy (GTK_WIDGET(data));
 	
 	GtkWidget *label;
	GtkWidget *button;
 	contain11 = gtk_vbox_new(0, 0);
 	
 	button = gtk_button_new_with_mnemonic("_Connect");
 	entry = gtk_entry_new();
	
	
	label = gtk_label_new("Enter your name:");
	gtk_box_pack_start(GTK_BOX(contain11), label, TRUE, 10, 0);
	gtk_widget_show(label);
	g_signal_connect(button, "clicked", G_CALLBACK(callback_connect), contain11);
	gtk_box_pack_start(GTK_BOX(contain11), entry, TRUE, 10, 0);
	gtk_box_pack_start(GTK_BOX(contain11), button, TRUE, 10, 0);
	gtk_widget_show(entry);
	gtk_widget_show(button);
	
	gtk_box_pack_start(GTK_BOX(contain1), contain11, TRUE, 10, 10);
	gtk_widget_show(contain11);
}

void callback_op (GtkButton * widget, gpointer data) {	
	const gchar *label = gtk_button_get_label (widget);
	struct msg *message = malloc(sizeof(struct msg));
	strcpy(message->buf, label);
	gtk_widget_destroy (GTK_WIDGET(contain));
	message->size = 0;
 	message->s = 0; 
	if (send(sockfd, message, sizeof(struct msg), 0) == -1) {
		perror("send");
		crt_end_menu(-3);
	}
	recv(sockfd, &game_number, sizeof(int), 0);
	pic = 0;
	numberpl = 2;
	player = 2;
	pl_cr();
}


void callback_jg (GtkWidget * widget, gpointer data)
{
	struct msg *message = malloc(sizeof(struct msg));
	if (connect_to_server() == -1)
		crt_end_menu(-3);
	
 	gtk_widget_destroy (GTK_WIDGET(data));
 	int i;
 	char one = 'y';
 	gchar name[nlen];
 	GtkWidget *button;
 	GtkWidget *label;
 	contain11 = gtk_vbox_new(0, 0);
 	
    buf[numbytes] = '\0';
    message->type = JOIN;
 	message->size = 0;
 	memset(message->buf, '\0', 100);
 	message->s = 0; 
	if (send(sockfd, message, sizeof(struct msg), 0) == -1) {
		perror("send");
		crt_end_menu(-3);
	}
	if (recv(sockfd, message, sizeof(struct msg), 0) == -1) { 
		perror("recv");
		crt_end_menu(-3);
	}
	if (message->size != 0){
		label = gtk_label_new("List of games:");
		gtk_box_pack_start(GTK_BOX(contain11), label, TRUE, 10, 0);
		gtk_widget_show(label);
		for(i = 1; i <= message->size; i++) {
			if ((numbytes = recv(sockfd, name, nlen-1, 0)) == -1) { 
				perror("recv");
				crt_end_menu(-3);
			}
			if (strlen(name) > 20 || strlen(name) < 4){
				crt_end_menu(-3);
			}
			if (send(sockfd, "y", strlen("y"), 0) == -1) {
					perror("send");
					crt_end_menu(-3);
			}
			button = (GtkButton *)gtk_button_new_with_label (name);
			
			gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (callback_op), NULL);
			gtk_box_pack_start(GTK_BOX(contain11), button, TRUE, 10, 0);
			gtk_widget_show (button);
		}
		gtk_box_pack_start(GTK_BOX(contain1), contain11, TRUE, 10, 10);
		gtk_widget_show(contain11);
	} else{
		label = gtk_label_new("Game not created");
		gtk_box_pack_start(GTK_BOX(contain11), label, TRUE, 10, 0);
		gtk_widget_show(label);
		button = gtk_button_new_with_label ("Create game");
		gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (callback_enter_name), contain11);
		gtk_box_pack_start(GTK_BOX(contain11), button, TRUE, 10, 0);
		gtk_widget_show (button);
		gtk_box_pack_start(GTK_BOX(contain1), contain11, TRUE, 10, 10);
		gtk_widget_show(contain11);
	}
}

void *get_in_addr(struct sockaddr *sa) 
{ 
    if (sa->sa_family == AF_INET) { 
        return &(((struct sockaddr_in*)sa)->sin_addr); 
	}

    return &(((struct sockaddr_in6*)sa)->sin6_addr); 
}

int connect_to_server()
{
    struct addrinfo hints, *servinfo, *p; 
    int rv; 
    char s[INET6_ADDRSTRLEN];


    memset(&hints, 0, sizeof hints); 
    hints.ai_family = AF_UNSPEC; 
    hints.ai_socktype = SOCK_STREAM; 
    int i;
    errno = ECONNREFUSED;
    for (i = 0; i < 3; i++){
		if (errno == ECONNREFUSED){
			if ((rv = getaddrinfo("127.0.0.1", PORT[i], &hints, &servinfo)) != 0) { 
				fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); 
				exit(1); 
			}

			for(p = servinfo; p != NULL; p = p->ai_next) { 
				if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) { 
					perror("client: socket"); 
					continue; 
				}
				if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) { 
					close(sockfd); 
					perror("client: connect"); 
					continue; 
				} else {
					errno = 0;
				}
				break;
			}
		}
	}
	if (p == NULL) { 
		fprintf(stderr, "client: failed to connect\n"); 
		return -1;
	} else{
		inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s); 
		freeaddrinfo(servinfo);
    }
    return 0;
}

void crt_main_menu()
{
	contain = gtk_vbox_new(0, 0);
	contain1 = gtk_vbox_new(0, 0);
	contain11 = gtk_vbox_new(0, 0);
	contain2 = gtk_vbox_new(0, 0);
	{
		button = gtk_button_new_with_label ("New game");

		gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (callback_enter_name), (gpointer) contain11);
		
		gtk_box_pack_start(GTK_BOX(contain11), button, TRUE, 10, 0);
		gtk_widget_show (button);
	}
	{
		button = gtk_button_new_with_label ("Join game");

		gtk_signal_connect (GTK_OBJECT (button), "clicked",
			GTK_SIGNAL_FUNC (callback_jg), (gpointer) contain11);
		
		gtk_box_pack_start(GTK_BOX(contain11), button, TRUE, 10, 0);
		gtk_widget_show (button);
	}
	
	gtk_box_pack_start(GTK_BOX(contain1), contain11, TRUE, 10, 10);
	gtk_widget_show (contain11);
	{
		button = gtk_button_new_with_label ("Quit");

		gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (delete_event), GTK_OBJECT (window));
		
		gtk_box_pack_start(GTK_BOX(contain2), button, TRUE, 10, 10);
		gtk_widget_show (button);
	}
	gtk_box_pack_start(GTK_BOX(contain), contain1, TRUE, 10, 10);
	gtk_box_pack_start(GTK_BOX(contain), contain2, TRUE, 10, 10);
	gtk_widget_show (contain1);
	gtk_widget_show (contain2);
	
	gtk_container_add (GTK_CONTAINER(window), contain);
	
	
	gtk_widget_show (contain);
	gtk_widget_show (window);
	
}

int main (int argc, char *argv[])
{
	gtk_init (&argc, &argv);
	
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	int i, j;
	
	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++)
			play_gr[i][j] = 0;
	
	gtk_window_set_title (GTK_WINDOW (window), "tic-tac-toe");
	gtk_signal_connect (GTK_OBJECT (window), "delete_event", GTK_SIGNAL_FUNC (delete_event), NULL);
	gtk_container_set_border_width (GTK_CONTAINER (window), 20);
	gtk_window_set_default_size(GTK_WINDOW(window), 325, 250);
	crt_main_menu();
	gtk_main();
	return 0;
}
