#define TRUE 	1
#define FALSE 	0

#define MAX_TAPE_NAME_LENGTH	30
#define MAX_DRIVE_TRIES		6
#define MAX_FILENAME_LENGTH	25

struct filename_ll {
	char			*filename;
	char			*tape_name;
	char			tape_format[4];
	int			file_num;
	char			*tar_path;
	struct filename_ll 	*next;
} ;

typedef struct filename_ll filename_s;

struct tape_ll {
	char			*tape_name;
	struct tape_ll 		*next;
} ;

typedef struct tape_ll tape_s;

void process_input_args();
void insert_first();
void print_filenames();
void print_tape_list();
void get_file_info();
void order_files();
void insert_in_order();
void extract_files();
void insert_tape();
void print_tapes();
void remove_tape();
void verify_files();

