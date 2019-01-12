***************************************************************************
*                         M S T A R  V I E W E R S                        *
***************************************************************************
*                               readme.txt                                *
***************************************************************************
*                                                                         *
*        Author: John F. Querns, Veridian Inc., Veda Operations           *
*          Date: 04 May 98                                                *
*                                                                         *
***************************************************************************
* THIS SOFTWARE AND ANY ACCOMPANYING DOCUMENTATION IS RELEASED "AS IS."   *
* THE U.S. GOVERNMENT MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,  *
* CONCERNING THIS SOFTWARE AND ANY ACCOMPANYING DOCUMENTATION, INCLUDING, *
* WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A  *
* PARTICULAR PURPOSE. IN NO EVENT WILL THE U.S. GOVERNMENT BE LIABLE FOR  *
* ANY DAMAGES, INCLUDING ANY LOST PROFITS, LOST SAVINGS OR OTHER          *
* INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE, OR INABILITY*
* TO USE, THIS SOFTWARE OR ANY ACCOMPANYING DOCUMENTATION, EVEN IF        *
* INFORMED IN ADVANCE OF THE POSSIBILITY OF SUCH DAMAGES."                *
***************************************************************************

This file describes the tools included in the MSTAR Viewer tools package
and how to extract them. The MSTAR Viewer toolkit consists of image viewer
code that can be used in two different software packages to view MSTAR 
target chip images and full scene files:

    a) Interface code for use with XV, a UNIX X Viewer developed by John
       Bradley of the University of Pennsylvania.  The XV portion of the
       MSTAR Viewer package includes C code that, when inserted into the
       XV software package, will allow XV to read and display MSTAR images.
       It also includes instructions for re-making the XV binaries.

    b) Matlab M-files for displaying single target chips, lists of target
       chips, and clutter full scenes.  The MATLAB portion of the MSTAR
       Viewer package includes two separate Matlab-based viewers: one
       for target chips and one for clutter scenes.  It also includes
       instructions for using the two tools.

The MSTAR Viewer tools are distributed in the following UNIX TAR archive:

    mstarviewers.tar    {UNIX TAR archive}


------------------
 EXTRACTING TOOLS
------------------

   1) [FOR PC USERS]: 

      % winzip mstarviewers.tar

        NOTE: Use of Winzip Wizard will generate an error stating
              that the above TAR file is not one of the valid
              ZIP formats.  However, you can view and extract
              the contents of this TAR file using Winzip Classic.

              To extract specific files, check Winzip Help.

      [FOR UNIX USERS]:

      [To extract ALL tools]:

        % tar xvf mstarviewers.tar 

      [To extract ONLY Matlab tools]:

        % tar xvf mstarviewers.tar matlabtools [enter]

   2) You should see the following files upon extraction:

      [If ALL tools extracted]:

       readme.txt            --  This file (again)

       xvmstar:              --  XV viewer code directory

           xv.txt            --  XV tools Readme
           br_mstar          --  X bitmap icon for XV browser
           xvmstar.c         --  C file with XV load module
                                 code for MSTAR files

       matlabtools:          --  MATLAB viewer tools directory

           matlab.txt        --  Matlab tools Readme

           viewchip:         --  Target chip viewer tool directory
                vwchip.m     --  M-file to display target chips
                rchplist.m   --  Function called by vwchip.m

           viewclut:         --  Clutter scene viewer tool directory
                vwclut.m     --  M-file to display clutter scenes

      [If ONLY Matlab tools extracted]:

       matlabtools:          --  MATLAB viewer tools directory

           matlab.txt        --  Matlab tools Readme

           viewchip:         --  Target chip viewer tool directory
                vwchip.m     --  M-file to display target chips
                rchplist.m   --  Function called by vwchip.m

           viewclut:         --  Clutter scene viewer tool directory
                vwclut.m     --  M-file to display clutter scenes


