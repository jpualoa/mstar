


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
#include <string.h>
#include "mstar_extract.h"


void
print_tapes (head)
     tape_s *head;
{

  while (head)
    {
      printf ("\t%s\n", head->tape_name);
      head = head->next;
    }
}



void
insert_tape (head, name)
     tape_s **head;
     char *name;
{
  tape_s *new_node, *ptr, *prev;
  int same;


  if (*head == NULL)
    {
      new_node = (tape_s *) malloc (sizeof (tape_s));
      new_node->tape_name = (char *) malloc (strlen (name));
      strcpy (new_node->tape_name, name);
      new_node->next = NULL;

      *head = new_node;
    }
  else
    {
      ptr = *head;
      prev = NULL;
      same = FALSE;
      while (ptr && !same)
	{
	  if (strcmp (ptr->tape_name, name))
	    {
	      prev = ptr;
	      ptr = ptr->next;
	    }
	  else
	    same = TRUE;
	}

      if (!same)
	{
	  new_node = (tape_s *) malloc (sizeof (tape_s));
	  new_node->tape_name = (char *) malloc (strlen (name));
	  strcpy (new_node->tape_name, name);
	  new_node->next = NULL;

	  prev->next = new_node;
	}
    }
}



void
remove_tape (head, name)
     tape_s **head;
     char *name;
{
  tape_s *ptr, *temp;

  if (!strcmp ((*head)->tape_name, name))
    *head = (*head)->next;
  else
    {
      ptr = *head;
      while (ptr->next)
	{
	  if (!strcmp ((ptr->next)->tape_name, name))
	    {
	      temp = ptr->next;
	      ptr->next = temp->next;
	      free (temp);
	    }
	}
    }
}
