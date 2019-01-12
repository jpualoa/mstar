/*******************************************************************/
/* THIS SOFTWARE AND ANY ACCOMPANYING DOCUMENTATION IS RELEASED "AS IS." */
/* THE U.S. GOVERNMENT MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, */
/* CONCERNING THIS SOFTWARE AND ANY ACCOMPANYING DOCUMENTATION, INCLUDING, */
/* WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A */
/* PARTICULAR PUPOSE.  IN NO EVENT WILL THE U.S. GOVERNMENT BE LIABLE FOR */
/* ANY DAMAGES, INCLUDING ANY LOST PROFITS, LOST SAVINGS OR OTHER        */
/* INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE, OR INABILITY */
/* TO USE, THIS SOFTWARE OR ANY ACCOMPANYING DOCUMENTATION, EVEN IF       */
/* INFORMED IN ADVANCE OF THE POSSIBILITY OF SUCH DAMAGES.                */
/************************************************************************/

#include <stdio.h>
#include "mstar_extract.h"

main (argc, argv)
     int argc;
     char *argv[];
{
  int tape_in_drive, number_of_files;
  char *tape_drive, *datafile;
  char *cmd = (char *) malloc (40);
  char *status = (char *) malloc (40);
  char ch;
  int ready = 0, i = 0;
  filename_s *files_to_extract, *ordered_files;
  FILE *check;


  tape_drive = NULL;
  files_to_extract = NULL;
  ordered_files = NULL;

  process_input_args (argc, argv, &tape_in_drive,
		      &tape_drive, &datafile);

  number_of_files = get_filenames_to_extract (&files_to_extract);
  get_file_info (files_to_extract, datafile);

  order_files (files_to_extract, &ordered_files);

  print_tape_list (ordered_files);

  sprintf (cmd, "mt -f %s status 2>&1", tape_drive);
  check = popen (cmd, "r");
  ch = fgetc (check);
  while (ch != '\n')
    {
      status[i++] = ch;
      ch = fgetc (check);
    }
  status[i] = '\0';
  pclose (check);

  if (strstr (status, "No such file") == NULL)
    {
      extract_files (ordered_files, tape_drive, tape_in_drive, datafile);
      verify_files (ordered_files);

    }
}

void
process_input_args (argc, argv, tape_in_drive,
		    tape_drive, datafile)
     int argc;
     char *argv[];
     int *tape_in_drive;
     char **tape_drive;
     char **datafile;
{

  int size;
  int i;
  int set = 0;


  if ((argc > 4) || (argc == 2 && !strcmp (argv[1], "-h")))
    {
      fprintf (stderr,
	"Usage:  mstar_extract tape_drive_device_name [-l msd_datafile]\n");
      exit (1);
    }

  /*  Initalize variable values and sizes */
  *tape_in_drive = FALSE;
  *datafile = (char *) malloc (15 + 1);
  strcpy (*datafile, "mstar_datafiles");
  *tape_drive = (char *) malloc (5 + 1);
  strcpy (*tape_drive, "dummy");

  i = 1;
  while (i < argc)
    {
      if (strcmp (argv[i], "-l") == 0)
	{
	  if (argc > i + 1)
	    {
	      size = strlen (argv[i + 1]);
	      *datafile = (char *) realloc (*datafile, size + 1);
	      strcpy (*datafile, argv[i + 1]);
	      i += 2;
	    }
	  else
	    i++;
	}
      else
	{
	  if (!set)
	    {
	      size = strlen (argv[i]);
	      *tape_drive = (char *) realloc (*tape_drive, size + 1);
	      strcpy (*tape_drive, argv[i]);
	      set = 1;
	    }
	  i++;
	}
    }
}


int
get_filenames_to_extract (filenames)
     filename_s **filenames;
{
  int rc, number_of_files;
  char name[255];

  number_of_files = 0;
  while (TRUE)
    {
      rc = get_line_stdin (name, 255);
      if (rc == EOF)
	break;
      insert_first (filenames, name);
      number_of_files++;
    }

  return number_of_files;
}



int
get_line_stdin (s, maxlen)
     char *s;
     int maxlen;
{
  int i, c;

  i = 0;
  while ((i < maxlen) && ((c = getchar ()) != EOF))
    {
      if (c == '\n')
	{
	  s[i] = '\0';
	  break;
	}
      else
	{
	  s[i++] = c;
	}
    }
  s[i] = '\0';
  if (c == EOF)
    return EOF;
  else
    return 0;
}



void
insert_first (head, name)
     filename_s **head;
     char *name;
{
  filename_s *new_node;


  new_node = (filename_s *) malloc (sizeof (filename_s));
  new_node->filename = (char *) malloc (strlen (name));
  strcpy (new_node->filename, name);

  if (*head == NULL)
    {
      new_node->next = NULL;
      *head = new_node;
    }
  else
    {
      new_node->next = *head;
      *head = new_node;
    }
}


void
print_filenames (head)
     filename_s *head;
{

  while (head)
    {
      printf ("\t%s  %s  %d\n",
	      head->filename, head->tape_name, head->file_num);
      head = head->next;
    }
}

void
print_tape_list (head)
     filename_s *head;
{
  char *tape = (char *) malloc (30);

  strcpy (tape, "First");

  while (head)
    {
      if (strcmp (tape, head->tape_name))
	{
	  printf ("\t%s\n",
		  head->tape_name);
	  strcpy (tape, head->tape_name);
	}
      head = head->next;
    }
}


void
get_file_info (files_to_extract, datafile)
     filename_s *files_to_extract;
     char *datafile;
{
  FILE *file_info_ptr;
  char str[80];
  char filename[80], tape_name[80], tape_format[4], tar_path[40];
  int file_num, rc, size;
  char junk[80];

  file_info_ptr = fopen (datafile, "r");
  if (file_info_ptr == NULL)
    {
      fprintf (stderr, "Error: file %s not found\n", datafile);
      exit (1);
    }
  close (file_info_ptr);

  while (files_to_extract)
    {
      sprintf (str, "grep \"%s \" %s",
	       files_to_extract->filename, datafile);
      file_info_ptr = popen (str, "r");
      if (file_info_ptr == NULL)
	{
	  fprintf (stderr, "Cannot spawn grep process: %s\n", str);
	  exit (1);
	}
      rc = fscanf (file_info_ptr, "%s %s %s %d",
		   filename, tape_name, tape_format, &file_num);

      if (rc < 4)
	{
	  fprintf (stderr, "%s NOT in MSTAR datafile '%s'\n",
		   files_to_extract->filename, datafile);
	  files_to_extract = files_to_extract->next;
	  continue;
	}
      else if (!strcmp (tape_format, "tar"))
	{
	  rc = fscanf (file_info_ptr, "%s", tar_path);
	  if (rc == 1)
	    {
	      size = strlen (tar_path);
	      files_to_extract->tar_path = (char *) malloc (size + 1);
	      strcpy (files_to_extract->tar_path, tar_path);
	    }
	  else
	    {
	      files_to_extract->tar_path = (char *) malloc (2);
	      strcpy (files_to_extract->tar_path, " ");
	    }
	}
      size = strlen (tape_name);
      files_to_extract->tape_name = (char *) malloc (size + 1);
      strcpy (files_to_extract->tape_name, tape_name);

      strcpy (files_to_extract->tape_format, tape_format);

      files_to_extract->file_num = file_num;

      files_to_extract = files_to_extract->next;
      pclose (file_info_ptr);
    }
}


void
order_files (files, ordered_files)
     filename_s *files;
     filename_s **ordered_files;
{
  filename_s *new_node;
  int size;



  while (files)
    {
      if (!files->tape_name)
	{
	  files = files->next;
	  continue;
	}
      new_node = (filename_s *) malloc (sizeof (filename_s));
      new_node->filename = (char *) malloc (strlen (files->filename));
      strcpy (new_node->filename, files->filename);

      size = strlen (files->tape_name);
      new_node->tape_name = (char *) malloc (size + 1);
      strcpy (new_node->tape_name, files->tape_name);

      if (!strcmp (files->tape_format, "tar"))
	{
	  size = strlen (files->tar_path);
	  new_node->tar_path = (char *) malloc (size + 1);
	  strcpy (new_node->tar_path, files->tar_path);
	}
      strcpy (new_node->tape_format, files->tape_format);

      new_node->file_num = files->file_num;

      insert_in_order (new_node, ordered_files);

      files = files->next;
    }
}


void
insert_in_order (node, head)
     filename_s *node;
     filename_s **head;
{
  int same_tape, done, tape_found;
  filename_s *ptr, *prev;



  if (*head == NULL)
    {
      node->next = NULL;
      *head = node;
    }
  else
    {
      ptr = *head;
      done = FALSE;
      prev = NULL;
      tape_found = FALSE;
      while (ptr && !done)
	{
	  same_tape = !strcmp (node->tape_name, ptr->tape_name);
	  if (tape_found && !same_tape)
	    {
	      node->next = ptr;
	      ptr = node;
	      prev->next = node;
	      done = TRUE;
	    }
	  else if (same_tape && (node->file_num < ptr->file_num))
	    {
	      if (prev)
		{
		  node->next = ptr;
		  ptr = node;
		  prev->next = node;
		}
	      else
		{
		  node->next = *head;
		  *head = node;
		}
	      done = TRUE;
	    }
	  else
	    {
	      prev = ptr;
	      ptr = ptr->next;
	    }
	  if (!tape_found)
	    tape_found = same_tape;
	}

      if (!done)
	{
	  node->next = NULL;
	  prev->next = node;
	}
    }
}




void
extract_files (files, drive, tape_in_drive, tape_name)
     filename_s *files;
     char *drive;
     int tape_in_drive;
     char *tape_name;
{
  filename_s *fptr;
  tape_s *tapes_to_be_read, *tptr;
  char current_tape[MAX_TAPE_NAME_LENGTH];
  char next_tape[MAX_TAPE_NAME_LENGTH];
  FILE *fp;
  char cmd[2048], junk[80];
  int file_num, block_num, tape_ready, tries;
  int key, file_diff, tape_file_pos;
  char filename[MAX_FILENAME_LENGTH];


  tapes_to_be_read = NULL;
  fptr = files;
  while (fptr)
    {
      strcpy (current_tape, fptr->tape_name);
      insert_tape (&tapes_to_be_read, current_tape);
      fptr = fptr->next;
      while (fptr)
	{
	  strcpy (next_tape, fptr->tape_name);
	  if (!strcmp (current_tape, next_tape))
	    fptr = fptr->next;
	  else
	    break;
	}
    }



  while (tapes_to_be_read)
    {
      if (tape_in_drive)
	strcpy (current_tape, tape_name);
      else
	{
	  strcpy (current_tape, tapes_to_be_read->tape_name);
	  printf ("Insert Tape '%s' into drive '%s'\n",
		  current_tape, drive);
	  system ("sleep 30");

	}

      remove_tape (&tapes_to_be_read, current_tape);


    /******************   CHECK IF DRIVE IS READY   ******************/
      tape_ready = FALSE;
      tries = 1;
      junk[0] = '\0';
      sprintf (cmd, "mt -f %s status", drive);
      while (!tape_ready)
	{
	  fp = popen (cmd, "r");
	  fgets (junk, 80, fp);
	  if (strcmp (junk, ""))
	    tape_ready = TRUE;
	  else
	    {
	      if (tries < MAX_DRIVE_TRIES)
		{
		  fprintf (stderr,
		      "Drive not ready, waiting 30 seconds to try again\n");
		  system ("sleep 30");
		  tries++;
		  pclose (fp);
		}
	      else
		{
		  fprintf (stderr,
		  "After %d tries, drive still not ready, exiting\n", tries);
		  pclose (fp);
		  exit (2);
		}
	    }
	}
    /*****************************************************************/

      pclose (fp);
      sprintf (cmd, "mt -f %s rewind", drive);
      system (cmd);
      tape_file_pos = 0;

      fptr = files;
      while (fptr && (strcmp (fptr->tape_name, current_tape)))
	fptr = fptr->next;

      while (fptr && (!strcmp (fptr->tape_name, current_tape)))
	{
	  file_num = fptr->file_num;
	  strcpy (filename, fptr->filename);
	  file_diff = file_num - tape_file_pos;

	  if (file_diff > 0)
	    {
	      sprintf (cmd, "mt -f %s fsf %d", drive, file_diff);
	      system (cmd);
	      tape_file_pos += file_diff;
	    }
	  if (!strcmp (fptr->tape_format, "dd"))
	    {
	      sprintf (cmd, "dd if=%s of=%s bs=8192", drive, filename);
	      system (cmd);
	      fptr = fptr->next;
	      tape_file_pos++;
	    }
	  else
	    {
	/*********************   TAR FORMAT   ********************/
	      sprintf (cmd, "tar xvf %s %s/%s ", drive, fptr->tar_path,
		       filename);
	      fptr = fptr->next;

	      while (fptr && !strcmp (fptr->tape_name, current_tape) &&
		     (fptr->file_num == file_num))
		{
		  strcat (cmd, fptr->tar_path);
		  strcat (cmd, "/");
		  strcat (cmd, fptr->filename);
		  strcat (cmd, " ");
		  fptr = fptr->next;
		}

	      system (cmd);

	/*********************************************************
	 **********************************************************
	 *   Solaris  will put the tape at the beginning of the   *
	 *   next file after a tar so this next statement is      *
	 *   necessary.  If you are running an OS that does not   *
	 *   advance the tape to the beginning of the next file   *
	 *   after a tar xvf, then remove this statement             *
	 **********************************************************
	 *********************************************************/
	      /* tape_file_pos++;      */

	/*********************************************************/
	    }

	}
      sprintf (cmd, "mt -f %s rewoffl", drive);
      system (cmd);
      tape_in_drive = FALSE;
    }

}






void
verify_files (files_to_check)
     filename_s *files_to_check;
{
  char cmd[80], filename[80];
  FILE *fp;


  while (files_to_check)
    {
      filename[0] = '\0';

      if (!strcmp (files_to_check->tape_format, "dd"))
	sprintf (cmd, "grep Filename %s", files_to_check->filename);
      else
	sprintf (cmd, "grep Filename %s/%s",
		 files_to_check->tar_path,
		 files_to_check->filename);

      fp = popen (cmd, "r");
      if (fp == NULL)
	{
	  fprintf (stderr, "Cannot spawn grep process\n");
	  exit (1);
	}
      fscanf (fp, "%*s %s", filename);
      pclose (fp);

      if (strcmp (filename, files_to_check->filename))
	printf ("File requested, %s was not file received, %s\n",
		files_to_check->filename, filename);
      else
	printf ("%s - OK\n", filename);

      files_to_check = files_to_check->next;

    }
}
