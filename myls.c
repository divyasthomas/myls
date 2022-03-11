#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>

#define Max_len_name 300

int maxLength_hardlink = 0;
int maxLength_inode = 0;
int maxLength_size = 0;
int Max_Len_hardlink_RegFile = 0;
int Max_Len_inode_RegFile = 0;
int Max_Len_size_RegFile = 0;

// print Owner of file and Group of owner
void Print_OwnerAndGroup(struct stat fStat) 
{            
  struct passwd* pw = getpwuid(fStat.st_uid);
  struct group* group = getgrgid(fStat.st_gid);

  if (pw == NULL) 
  {
    printf("No name found for %u\n", fStat.st_uid);
  }
  else if (group == NULL) 
  {
    printf("No group name for %u found\n", fStat.st_gid);
  }
  else {
    printf("%s %s ", pw->pw_name, group->gr_name);
  }
}

 // print time & date 
void Print_datetime(struct stat fStat) {        
  struct tm* t = localtime(&fStat.st_mtime);
  int yr = t->tm_year + 1900;
  const char * mon[] = { "Jan", "Feb","Mar", "Apr","May", "Jun", "Jul","Aug","Sep","Oct","Nov","Dec"};
  int day = t->tm_mday;
  int hr = t->tm_hour;
  int min = t->tm_min;
  printf("%s %2d %d %02d:%02d ", mon[t->tm_mon], day, yr, hr, min);
}


// printing the Permissions 
void Print_Permissions(struct stat fStat) 
{            
	//if link or directory 
	if (S_ISDIR(fStat.st_mode)) 
  {
    printf("%c",'d');                                          
  } 
  else if (S_ISLNK(fStat.st_mode)) 
  {
    printf("%c",'l');                                         
  }

  else 
  {
    printf("%c",'-');                                          
  }
  
  //rest of permissions
  printf( "%c", (fStat.st_mode & S_IRUSR) ? 'r' : '-');       
  printf( "%c", (fStat.st_mode & S_IWUSR) ? 'w' : '-');
  printf( "%c", (fStat.st_mode & S_IXUSR) ? 'x' : '-');
  printf( "%c", (fStat.st_mode & S_IRGRP) ? 'r' : '-');
  printf( "%c", (fStat.st_mode & S_IWGRP) ? 'w' : '-');
  printf( "%c", (fStat.st_mode & S_IXGRP) ? 'x' : '-');
  printf( "%c", (fStat.st_mode & S_IROTH) ? 'r' : '-');
  printf( "%c", (fStat.st_mode & S_IWOTH) ? 'w' : '-');
  printf( "%c", (fStat.st_mode & S_IXOTH) ? 'x' : '-');
}



int Get_Max_Lengths_for_Formatting(char* entry_name, char** dir_file_names, int file_count)
{
	
	
   for (int i = 0; i < file_count; i++) 
   {
		 struct stat fStat;
		 char fullPath[Max_len_name];
		 strcpy(fullPath, entry_name);
		 strcat(fullPath, "/");
		 strcat(fullPath, dir_file_names[i]);
		 
		 // handling error accessing file stat
		 if (lstat(fullPath, &fStat) < 0) 
		 {                   
		   printf("Error: Unable to get the file information using lstat..\n");
		   return -1;
		 }

// get number of hard links for file
		 long n = fStat.st_nlink;                                 
		 int count = 0;
		 while (n != 0) 
		 {
		   n /= 10;
		   count++;
		 }

		 if (count > maxLength_hardlink) 
		 {                                   
		   maxLength_hardlink = count;
		 }
	
// get the inode number	 
		  n = fStat.st_ino;                                   
		 count = 0;
		 while (n != 0) 
		 {
		   n /= 10;
		   count++;
		 }

		 if (count > maxLength_inode) 
		 {                                    
		   maxLength_inode = count;
		 }	 
		 
 // getting file size	 
	n = fStat.st_size;                                 
    count = 0;
    while (n != 0) 
    {
      n /= 10;
      count++;
    }

    if (count > maxLength_size) 
    {                                    
      maxLength_size = count;
    }
		 
		 
  }

  return 0;
	
}


// if name contains special char or whitespace- return true
bool has_special_characters(char* name) {         
  if (strchr(name, ' ') != NULL || strchr(name, '$') != NULL || strchr(name, '<') != NULL || strchr(name, '>') != NULL || strchr(name, '\'') != NULL ||  strchr(name, '^') != NULL || strchr(name, ':') != NULL || strchr(name, '!') != NULL || strchr(name, '&') != NULL || strchr(name, ',') != NULL) 
  {
    return true;
  }
  else {
    return false;
  }
}



char* clean_str(char* str) 
{                   
  int str_len = 0;
  char* clean_str = malloc(str_len * sizeof(char));
  for (int i = 0; i < strlen(str); i++) 
  {
    if (i != 0 && str[i - 1] == '/' && str[i] == '/') 
    {
      continue;
    }
    else 
    {
      clean_str = (char*)realloc(clean_str, (++str_len) * sizeof(char));
      clean_str[str_len - 1] = str[i];
    }
  }

  clean_str = (char*)realloc(clean_str, (++str_len) * sizeof(char));
  //adding term char
  clean_str[str_len - 1] = '\0';
  return clean_str;
}

//prints file name checking if having special char or not
void Print_filename(char* myls_Opt,char* f_name)
{

  // printing file with special characters

  if (has_special_characters(f_name)) 
  {                             
    char* clean_fname = clean_str(f_name);
    if (strchr(myls_Opt, 'l') == NULL) 
    {
    	//printf("no l\n");
      printf("\'%s\'", clean_fname);
    }
    else
    {
		 char buffer[Max_len_name];
		 //printf("l is there\n");
		 //symb link
		 ssize_t  bytes_read = readlink(f_name , buffer , sizeof(buffer));
		 //printf("bytes_read is%ld",bytes_read);
		 if (bytes_read != -1)
		 { //printf("soft l is there\n");
		   buffer[bytes_read] = '\0'; //adding term ch
		   printf("\'%s\' -> %s",clean_fname, buffer);
		 }
		 else
		   printf("\'%s\'", clean_fname);
    }
    free(clean_fname);
  }
  
  // printing file without special characters
  else 
  {
    char* clean_fname = clean_str(f_name);
    if (strchr(myls_Opt, 'l') == NULL) 
    { //printf("no l-- no sp\n");
      printf("%s", clean_fname);
    }
    else 
    { //printf("l is there\n");
      char buffer[Max_len_name];
      //symb link
      ssize_t  bytes_read = readlink(f_name , buffer , sizeof(buffer));
      //printf("bytes_read is%ld",bytes_read);
      if (bytes_read != -1)
      { //printf("soft l is there\n");
        buffer[bytes_read] = '\0';
        printf(" %s -> %s",clean_fname, buffer);
      }
      else
        printf(" %s", clean_fname);
    }

    free(clean_fname);
  }
 }


// Sort by ASCII and case sensitive
void LexSort(char** str_arr, int len) 
{ 
	/*
	for (int i = 0; i < len; i++) 
	  { 
		 printf("trace.. %s\n",str_arr[i]);
	  }
	 */        
  for (int i = 0; i < len- 1; i++) 
  {                        
    for (int j = i + 1; j < len; j++) 
    {
      if (strcmp(str_arr[i], str_arr[j]) > 0) 
      {
      //printf("trace!! printing str_arr[i],str_arr[j] %s %s\n",str_arr[i],str_arr[j]);
        char* temp = str_arr[i];
        str_arr[i] = str_arr[j];
        str_arr[j] = temp;
      }
    }
  }
}



// printing files by Directory
int Print_Dir_Info(char* myls_Opt,int count_file_list, char* entry_name ) 
{     
  DIR* dir = opendir(entry_name);
  if (dir == NULL)
  {
    return -1;
  }
  int file_count = 0; int dir_count = 0;
  
  char** dir_file_names;
  dir_file_names = malloc(file_count * sizeof(char*));
  char** dir_dir_names;
  dir_dir_names = malloc(dir_count * sizeof(char*));
  
  //reading dir
  struct dirent* cur_dir = readdir(dir);
  while (cur_dir != NULL) 
  {
    if (cur_dir->d_name[0] != '.') 
    {   
      dir_file_names = (char**)realloc(dir_file_names, (++file_count) * sizeof(char*));
      dir_file_names[file_count - 1] = cur_dir->d_name;
    }

    cur_dir = readdir(dir);
  }
  
   //find max_lens for formatting                                  
  int ret=Get_Max_Lengths_for_Formatting(entry_name, dir_file_names,  file_count);
  if (ret == -1 ) 
  { return -1; }
  
// Sorting the files in the directory
  LexSort(dir_file_names, file_count); 
  


//print directory name if recursive or more than 1 directory
  if (strchr(myls_Opt, 'R') != NULL || count_file_list > 1) 
  {     
    if (has_special_characters(entry_name)) 
    { // if filename contains special characters
      char* clean_name = clean_str(entry_name);
      printf("\'%s\':\n", clean_name);
      free(clean_name);
    }
    else 
    {
      char* clean_name = clean_str(entry_name);
      printf("%s:\n", clean_name);
      free(clean_name);
    }
  }


  //printing out file info
  for (int i = 0; i < file_count; i++) 
  {
    struct stat fStat;
    char fullPath[Max_len_name];
    strcpy(fullPath, entry_name);
    strcat(fullPath, "/");
    strcat(fullPath, dir_file_names[i]);
    if (lstat(fullPath, &fStat) < 0)
    {
       return -1;
    }
	//print out inode info if option i present
    if (strchr(myls_Opt, 'i') != NULL) 
    {                                   
      printf("%*ld ", maxLength_inode, fStat.st_ino);
    }
	//print long listing format if l is present
    if (strchr(myls_Opt, 'l') != NULL)
    {                                   
      Print_Permissions(fStat);                                            
      printf("%*ld ", maxLength_hardlink, fStat.st_nlink);                
      Print_OwnerAndGroup(fStat);                                          
      printf("%*ld ", maxLength_size, fStat.st_size);                 
      Print_datetime(fStat);                                       
    }

	//printing out file names 
	Print_filename(myls_Opt,dir_file_names[i]);

    //formatting output
    if (strchr(myls_Opt, 'i') != NULL)
 	 {              
   	printf("   \n");                                      
 	 }
 	 else
 	 {
    	printf("\n");
  	 }
    
//checking if item is a directory, if so adding to list of directories
    
    if (S_ISDIR(fStat.st_mode)) 
    {                                       
      char* fullPath_dyn = malloc(Max_len_name * sizeof(char));
      strcpy(fullPath_dyn, entry_name);
      strcat(fullPath_dyn, "/");
      strcat(fullPath_dyn, dir_file_names[i]);
      dir_dir_names = (char**)realloc(dir_dir_names, (++dir_count) * sizeof(char*));
      dir_dir_names[dir_count - 1] = fullPath_dyn;
    }
  }

//if R is selected going depth first into that directories inside
  if (strchr(myls_Opt, 'R') != NULL) 
  {                                    
    for (int i = 0; i < dir_count; i++) {
      printf("\n");
      Print_Dir_Info(myls_Opt,count_file_list, dir_dir_names[i]);
    }
  }

  closedir(dir);
  
  //freeing
  free(dir_file_names);
  for (int i = 0; i < dir_count; i++) {
    free(dir_dir_names[i]);
  }

  free(dir_dir_names);
  return 0;
}

int main(int argc, char ** argv)
{

  char myls_Opt[20];
  strcpy(myls_Opt, "-");
  
  char** myls_FileNames;
  int count_file_list = 0;
  myls_FileNames = malloc(count_file_list * sizeof(char*));
  myls_FileNames = (char**)realloc(myls_FileNames, (++count_file_list) * sizeof(char*));
  myls_FileNames[count_file_list - 1] = ".";
  bool in_file_list = 0;
 
 //more than one arg 
  if (argc > 1)
   {
    for (int i = 1; i < argc; i++) 
    {                                  
		//is it an option? 
      if (argv[i][0] == '-' && in_file_list == 0)
       { 
        strcat(myls_Opt, argv[i]);
        //printf("trace!! printing myls options%s\n",myls_Opt);
      }
		else 
		{
        if (in_file_list == 1) 
        {
          myls_FileNames = (char**)realloc(myls_FileNames, (++count_file_list) * sizeof(char*));
        }
        in_file_list=1;
		  //printf("trace-current argv[i] is %s\n",argv[i]);
        myls_FileNames[count_file_list - 1] = argv[i];
        
      }
    }
  }

// Error handling if User specified an unsupported option
  if (strcmp(myls_Opt, "-") != 0) 
  {                      
    for (int i = 0; i < strlen(myls_Opt); i++) 
    {
      if (myls_Opt[i] != '-' && myls_Opt[i] != 'i' && myls_Opt[i] != 'R' && myls_Opt[i] != 'l') 
      {
        printf("Error: User specified an unsupported option. Available options are  only three: i, R, l.. Exiting.. \n");
        free(myls_FileNames);
        exit(-1);
      }
    }
  }

/*
for (int i = 0; i < count_file_list; i++) 
  { 
    printf("trace.. %s\n",myls_FileNames[i]);
  }
*/

// calling sort
	//printf("trace! calling sort...\n");
  LexSort(myls_FileNames, count_file_list); 
  
 //find maxvals for formatting output 
  for (int i = 0; i < count_file_list; i++) 
  { 
    struct stat fStat;
    
    //Error handling if User specified specified a nonexistent file/directory
    if (lstat(myls_FileNames[i], &fStat) < 0) 
    {
      printf("Error: User might have specified a nonexistent file/directory. Unable to get the file/directory information.. Exiting..\n");
      exit(-1);
    }

	//is the entry a file? if so getting file info to find maxvals for formatting
    if (S_ISREG(fStat.st_mode) > 0) 
    { 
    	//Number of hard links 
      long n = fStat.st_nlink;
      //printf("trace: hardlinks %ld\n",n);
      int Count = 0;
      while (n != 0) {
        n =n/10;
        Count++;
      }

      if (Count > Max_Len_hardlink_RegFile)
      {
        Max_Len_hardlink_RegFile = Count;
        //printf("trace: Max_Len_hardlink_RegFile %d\n",Max_Len_hardlink_RegFile);
      }
		
		// Inode number 
      n = fStat.st_ino;
       //printf("trace: Inode number %ld\n",n);
      Count = 0;
      while (n != 0) 
      {
        n =n/10;
        Count++;
      }

      if (Count > Max_Len_inode_RegFile) 
      {
        Max_Len_inode_RegFile = Count;
        //printf("trace: Max_Len_inode_RegFile %d\n",Max_Len_inode_RegFile);
      }
	
		// Total size, in bytes 
		n = fStat.st_size;
		//printf("trace: Total size%ld\n",n);
      Count = 0;
      while (n != 0) 
      {
       n =n/10;
        Count++;
      }

      if (Count > Max_Len_size_RegFile) 
      {
        Max_Len_size_RegFile = Count;
        //printf("trace: Max_Len_size_RegFile %d\n",Max_Len_size_RegFile);
      }
    }
  }

   // checking if file or dir- if file, printing file info
  bool file_present = false;
  for (int i = 0; i < count_file_list; i++) 
  {
    DIR* dir = opendir(myls_FileNames[i]);
    if (dir == NULL) 
    { // if dir is null, then not directory
    	//printf("trace: not dir..\n");
      file_present = true;
      // Printing out file info
      
      // print file info
                          
	  struct stat fStat;
	  if (lstat(myls_FileNames[i], &fStat) < 0) 
	  {
		   printf("Error: User might have specified a nonexistent file/directory. Unable to get the file/directory information.. Exiting..\n");
		  		exit(-1);
	  }
		//printing inode if requested
	  if (strchr(myls_Opt, 'i') != NULL) 
	  {                                 
		 printf("%*ld ", Max_Len_inode_RegFile, fStat.st_ino);
	  }
	  
	//printing long format if requested
	  if (strchr(myls_Opt, 'l') != NULL) 
	  {                                 
		 Print_Permissions(fStat);                                          
		 printf("%*ld ", Max_Len_hardlink_RegFile, fStat.st_nlink);     
		 Print_OwnerAndGroup(fStat);                                        
		 printf("%*ld ", Max_Len_size_RegFile, fStat.st_size);       
		 Print_datetime(fStat);                                    
	  }
	  
	  //printing file name
	  Print_filename(myls_Opt,myls_FileNames[i]);

	  printf("\n");

                                
    }
    
    closedir(dir);
  }
  
//printing directory info
  for (int i = 0; i < count_file_list; i++) 
  {
    DIR* dir = opendir(myls_FileNames[i]);
    if (dir != NULL) 
    { // Check if its directory
		 
		 //if file info was printed out before..then entering newline before printing directory info like in ls
		 if (file_present) 
		 {
		   printf("\n");
		   file_present = false;
		 }
			
			//printf("trace:in_file_list..%d\n",in_file_list);
			//printf("trace:count_file_list..%d\n",count_file_list);
     
     int  dir_ret = Print_Dir_Info(myls_Opt, count_file_list ,myls_FileNames[i] ); 
     if (dir_ret ==-1)
     {
		  printf("Error: No such file or directory\n");
		  exit(-1);
     }
      if (i != count_file_list - 1)
      {
        printf("\n");
      }                                    
    }
    closedir(dir);
  }

  free(myls_FileNames);
  return 0;
}
