/*  This file is part of vobcopy.
 *
 *  vobcopy is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  vobcopy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with vobcopy; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#include "vobcopy.h"

/* included from cdtitle.c (thx nils *g)
 * get the Title
 * returns 0 on success or a value < 0 on error
 */
int get_dvd_name(const char *device, char *title)
{

#if defined( __sun )
  /* title is actually in the device name */
  char *new_title;
  new_title = strstr( device, "d0/" ) + strlen( "d0/" );
  strncpy( title, new_title, 255 );
#else
  int  filehandle = 0;
  int  i = 0, last = 0;
  int  bytes_read;

  char tmp_buf[2048];

       
  /* open the device */
  if ( !(filehandle = open(device, O_RDONLY, 0)) )       
  {
      /* open failed */
      fprintf( stderr, "[Error] Something went wrong while getting the dvd name  - please specify path as /cdrom or /dvd (mount point) or use -t\n");
      fprintf( stderr, "[Error] Opening of the device failed\n");
      fprintf( stderr, "[Error] Error: %s\n", strerror( errno ) );
      return -1;
  }
  
  /* seek to title of first track, which is at (track_no * 32768) + 40 */
//  if ( 32808 != lseek( filehandle, 32808, SEEK_SET ) ) 
  if ( 32768 != lseek( filehandle, 32768, SEEK_SET ) )      
  {
      /* seek failed */
      close( filehandle );
      fprintf( stderr, "[Error] Something went wrong while getting the dvd name - please specify path as /cdrom or /dvd (mount point) or use -t\n");
      fprintf( stderr, "[Error] Couldn't seek into the drive\n");
      fprintf( stderr, "[Error] Error: %s\n", strerror( errno ) );
      return -1;
  }
  
  /* read title */
//  if ( (bytes_read = read(filehandle, title, 32)) != 32) 
  if ( (bytes_read = read(filehandle, tmp_buf, 2048)) != 2048 )      
  {
      close(filehandle);
      fprintf( stderr, "[Error] something wrong in dvd_name getting - please specify path as /cdrom or /dvd (mount point) or use -t\n" );
      fprintf( stderr, "[Error] only read %d bytes instead of 2048\n", bytes_read);
      fprintf( stderr, "[Error] error: %s\n", strerror( errno ) );
      return -1;
  }
  
  strncpy( title, &tmp_buf[40], 32 ); 
//   memcpy(title, help + 40, 32);
   
  /* make sure string is terminated */
  title[32] = '\0';
  
  /* we don't need trailing spaces           */
  /* find the last character that is not ' ' */
  last = 0; /* and checkski below if it changes*/
  for ( i = 0; i < 32; i++ ) 
  {
      if ( title[i] != ' ' ) { last = i; }
  }
  if( 0 == last )
      {
          fprintf( stderr, "[Hint] The dvd has no name, will choose a nice one ;-), else use -t\n" );
          strcpy( title, "insert_name_here\0" );
      }
  else
      title[ last + 1 ] = '\0';

#endif
  return 0;
}


/* function to get the device of a given pathname */
/* returns 0 if successful and NOT mounted        */
/*         1 if successful and mounted            */
/* returns <0 if error                            */
int get_device( char *path, char *device )
{

#if ( !defined( __sun ) )
  FILE	*tmp_streamin;
  char	tmp_bufferin[ MAX_STRING ];
  char  tmp_path[ 256 ];
  int   l = 0;

#endif

#if (defined(__linux__))
  struct mntent* lmount_entry;
#endif

  FILE  *mnttab_fp;
  char  *pointer;
  char *new_device, *mnt_special;
  char  *k;
  bool  mounted = FALSE;
  int mntcheck;
#ifdef USE_STATFS_FOR_DEV
  struct statfs buf;
#endif
#ifdef USE_STATVFS_FOR_DEV
  struct statvfs buf;
#endif


  /* the string should have no trailing / */
  if( path[ ( strlen( path ) - 1 ) ] == '/' )
  {
      path[ ( strlen( path ) - 1 ) ] = '\0' ;
  }

  /* remove video_ts if given */
  if( ( pointer = strstr( path, "/video_ts" ) ) || 
      ( pointer = strstr( path, "/VIDEO_TS" ) ) )
  {
      *pointer = '\0';
  }

  /* check if it is given as /dev/xxx already */
  if( strstr( path, "/dev/" ) )
  {
      strcpy( device, path );
  }
  else
  {

    /*
     *look through /etc/mtab to see if it's actually mounted
     */
#if ( defined( USE_STATFS_FOR_DEV ) || defined( USE_STATVFS_FOR_DEV ) ) 

#ifdef USE_STATFS_FOR_DEV
    if( !statfs( path, &buf ) )
#else
    if( !statvfs( path, &buf ) )
#endif
      {
       if( !strcmp( path, buf.f_mntonname ) )
         {
           mounted = TRUE;
#if defined(__FreeBSD__) && (__FreeBSD_Version > 500000)
          strcpy(device, buf.f_mntfromname);
#else
	   strcpy(device, "/dev/r");
	   strcat(device, buf.f_mntfromname + 5);
#endif
	   return mounted;
         }
         strcpy(device, buf.f_mntfromname);
      }
    else
      {
       fprintf( stderr, "[Error] Error while reading filesystem info" );
       return -1;
      }
#elif ( defined( __sun ) )

    struct mnttab mount_entry;
    int           mntcheck;
 
    if ( ( mnttab_fp = fopen( "/etc/mnttab", "r" ) ) == NULL )
      {
	fprintf( stderr, " [Error] Could not open mnttab for searching!\n" );
	fprintf( stderr, " [Error] error: %s\n", strerror( errno ) );
	return -1;
      }

    while ( ( mntcheck = getmntent( mnttab_fp, &mount_entry ) ) == 0 )
      {
	/* check to see if our path is this entry */
	if ( strcmp( path, mount_entry.mnt_mountp ) == 0 )
	  {
	    char *new_device, *mnt_special;
	    if ( strstr( mount_entry.mnt_special, "/dsk/" ) == NULL )
	      {
		fprintf( stderr, "[Error] %s doesn't look like a disk device to me",
			 mount_entry.mnt_special );
		return -1;
	      }
	    /* we actually want the raw device name */

	    mnt_special = malloc( strlen( mount_entry.mnt_special ) + 1 );
	    new_device = malloc( strlen( mount_entry.mnt_special ) + 2 );
	    strcpy( mnt_special, mount_entry.mnt_special );
	    strcpy( new_device, mnt_special );
	    strcpy( strstr( new_device, "/dsk/" ), "" );
	    strcat( new_device, "/rdsk/" );
	    strcat( new_device,
		    strstr( mnt_special, "/dsk/" ) + strlen( "/dsk/" ) );
	    strncpy( device, new_device, 255 );
	    free( mnt_special );
	    free( new_device );
	    mounted = TRUE;
	    break;
	  }
      }
    if ( mntcheck > 0 )
      {
         fprintf( stderr, "[Error] Encountered error in reading mnttab file\n" );
         fprintf( stderr, "[Error] error: %s\n", strerror( errno ) );
         return -1;
      }
    else if ( mntcheck == -1 )
      {
         fprintf( stderr, "[Error] Did not find mount %s in mnttab!\n", path );
         return -1;
      }
#else
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
this is the code for the other-OSs, not solaris*/

#if (defined(__linux__)) 
    if ((tmp_streamin = setmntent("/etc/mtab", "r"))){

	while ((lmount_entry = getmntent(tmp_streamin))){
	    if (strcmp(lmount_entry->mnt_dir, path) == 0){
		/* Found the mount point */
		fprintf ( stderr, "[Info] Device %s mount on %s\n", lmount_entry->mnt_dir,
			lmount_entry->mnt_fsname);
		strcpy(device, lmount_entry->mnt_fsname);
		mounted = TRUE;
		break;
	    }
	}
	endmntent(tmp_streamin);
	if (mounted) 
            {
		/* device was set from /etc/mtab, no need to further check
		 * /etc/fstab */
		return mounted;
            }
    }
#endif

    
    if( ( tmp_streamin = fopen( "/etc/mtab", "r" ) ) )
      {
	strcpy( tmp_path, path );
	strcat( tmp_path, " " ); /* otherwise it would detect that e.g. 
				  /cdrom is mounted even if only/cdrom1 is 
				  mounted */ 

	memset( tmp_bufferin, 0, MAX_STRING * sizeof( char ) );
	while( fgets( tmp_bufferin, MAX_STRING, tmp_streamin )) 
          {
	    if( strstr( tmp_bufferin, tmp_path ) )
              {
		mounted = TRUE;
	      }
	  }
	fclose( tmp_streamin );
      }
    else
      {
	fprintf( stderr, "[Error] Could not read /etc/mtab!\n" );
	fprintf( stderr, "[Error] error: %s\n", strerror( errno ) );
	return -1;
      }  
#endif

#if defined( __sun )
  }
#else

    
    /*
     *read the device out of /etc/fstab 
     */

    if( ( tmp_streamin = fopen( "/etc/fstab", "r" ) ) )
      {
	strcpy(tmp_path, path);

	memset( tmp_bufferin, 0, MAX_STRING * sizeof( char ) );
	while( fgets( tmp_bufferin, MAX_STRING, tmp_streamin ) ) 
	  {
	    if( ( pointer = strstr( tmp_bufferin, tmp_path ) ) )
              {
		if( isgraph( ( int ) *( pointer + strlen( tmp_path ) ) ) )
		  break; /* there is something behind the path name, 
			    for instance like it would find /cdrom but 
			    also /cdrom1 since /cdrom is a subset of /cdrom1 */
		/*isblank should work too but how do you do that with 
		  the "gnu extension"? (man isblank) */

		if( ( k = strstr( tmp_bufferin, "/dev/" ) ) == NULL )
		  {
		    fprintf( stderr, "[Error] Weird, no /dev/ entry found in the line where iso9660 or udf gets mentioned in /etc/fstab\n" );
		    return -1;
		  }
		l=0;

		while( isgraph( (int) *(k) ) )
	          {
		    device[l] = *(k);
		    if( device[l] == ',' )
		      break;
		    l++;
		    k++;
	          }
		if( isdigit( ( int) device[l-1] ) )
	          {
                 if( strstr( device, "hd" ) )
	              fprintf(stderr, "[Hint] Hmm, the last char in the device path (%s) that gets mounted to %s is a number.\n", device, path);
	          }
		device[l] = '\0';
	      }
	    memset( tmp_bufferin, 0, MAX_STRING * sizeof( char ) );
          }
          fclose( tmp_streamin );
	  if( !strstr( device, "/dev" ) )
	    {
	      fprintf(stderr, "[Error] Could not find the provided path (%s), typo?\n",path);
	      device[0] = '\0';
	      return -1;
	    }
      }
    else
      {
	fprintf( stderr, "[Error] Could not read /etc/fstab!" );
	fprintf( stderr, "[Error] error: %s\n", strerror( errno ) );
	device[0] = '\0';
	return -1;
      }
  }
#endif
  return mounted;
}


/* function to get the device WITHOUT a given pathname */
/* therefore called oyo - on your own */
/* returns 0 if successful and NOT mounted        */
/*         1 if successful and mounted            */
/* returns <0 if error                            */
int get_device_on_your_own( char *path, char *device )
{ /*oyo*/
#ifdef USE_GETMNTINFO
  int i, n, dvd_count = 0;
#ifdef GETMNTINFO_USES_STATFS
  struct statfs *mntbuf;
#else
  struct statvfs *mntbuf;
#endif

  if( ( n = getmntinfo( &mntbuf, 0 ) ) > 0 )
    {
      for( i = 0; i < n; i++ )
        {
          if( strstr( mntbuf[i].f_fstypename, "cd9660" ) || strstr( mntbuf[i].f_fstypename, "udf" ) )
            {
              dvd_count++;
              strcpy( path, mntbuf[i].f_mntonname );
#if defined(__FreeBSD__) && (__FreeBSD_Version > 500000)
             strcat(device, mntbuf[i].f_mntfromname);
#else
	      strcpy(device, "/dev/r");
	      strcat(device, mntbuf[i].f_mntfromname + 5);
#endif
            }
        }
      if(dvd_count == 0)
        { /* no cd found? Then user should mount it */
	  fprintf(stderr, "[Error] There seems to be no cd/dvd mounted. Please do that..\n");
	  return -1;
        }
    }
  else
    {
      fprintf(stderr, "[Error] An error occured while getting mounted file system information\n");
      return -1;
    }

#elif ( defined( __sun ) )

    /* Completely rewritten search of a mounted DVD on Solaris, much
     cleaner. No more parsing of strings -- lb */

    // the mnttab file
    FILE         *mnttab_fp;
    // The mount entry found
    struct mnttab mountEntry;
    // The mount search fields
    struct mnttab udfsMount;
    // Number of found entries
    int           dvd_count = 0;
    // Result of the mount search
    int           result    = 0;


    /* We're looking for any udfs-mounted FS
       Note that this is not always a DVD */
    udfsMount.mnt_mountp  = NULL;
    udfsMount.mnt_special = NULL;
    udfsMount.mnt_fstype  = "udfs";
    udfsMount.mnt_mntopts = NULL;
    udfsMount.mnt_time    = NULL;
	   
    /* Try to open the mnttab info */
    if (( mnttab_fp = fopen( "/etc/mnttab", "r" ) ) == NULL) {
      fprintf( stderr, "[Error] Could not open mnttab for searching!\n" );
      fprintf( stderr, "[Error] error: %s\n", strerror( errno ) );
      return -1;
	     }

    // Get the matching mount points
    // If there is a match, handle it
    while ((result = getmntany(mnttab_fp, &mountEntry, &udfsMount)) == 0) {
      dvd_count++;
	 }

    // If the last result is not -1, there was a problem -- exit
    if (result != -1)
      return -1;
   
    // no cd found? Then user should mount it
    if (dvd_count == 0) {
	 fprintf(stderr, "[Error] There seems to be no cd/dvd mounted. Please do that..\n");
	 return -1;
       }

    // Copy the device name and mount points to the output variables
    strcpy(path,   mountEntry.mnt_mountp);
    strcpy(device, mountEntry.mnt_special);

#else /* End ifdef(__sun) */

  FILE	*tmp_streamin;
  char	tmp_bufferin[MAX_STRING];
/*  char  tmp_path[20]; */
  int   l = 0, dvd_count = 0;
  char *k;
  /*
   *read the device out of /etc/mtab
   */

 if( ( tmp_streamin = fopen( "/etc/mtab", "r" ) ) )
   {
/*      strcpy(tmp_path, "iso9660"); */
     memset( tmp_bufferin, 0, MAX_STRING * sizeof( char ) );
     while( fgets( tmp_bufferin, MAX_STRING, tmp_streamin ) ) 
     {
/*        if(strstr( tmp_bufferin, tmp_path)) */
       if (strstr( tmp_bufferin, "iso9660" ) || 
           strstr( tmp_bufferin, "udf" )     || 
           strstr( tmp_bufferin, "cdrom" )   || 
           strstr( tmp_bufferin, "dvd" ) )
	 {
	   dvd_count++; /*count every cd we find */
	   /*
	     extract the device
	   */

	   if( ( k = strstr( tmp_bufferin, "/dev/" ) ) == NULL )
	     {
	       fprintf( stderr, "[Error] Weird, no /dev/ entry found in the line where iso9660, udf or cdrom gets mentioned in /etc/mtab\n" );
	       return -1;
	     }

	   while(isgraph( (int) *(k) ))
	     {
	       device[l] = *(k);
	       if( device[l] == ',' )
		 break;
	       l++;
	       k++;
	     }
	   device[l] = '\0';
	   if(isdigit((int)device[l-1]))
	     {
                 if( strstr( device, "hd" ) )
                    fprintf(stderr, "[Hint] Hmm, the last char in the device path %s is a number.\n", device );
	     }
	   
	   /*The syntax of /etc/fstab and mtab seems to be something like this:
	    Either 
	    /dev/hdc /cdrom
	    or, in case of supermount
	    none /mnt/cdrom supermount ro,dev=/dev/hdc,
	    Therefore I parse for /dev/ first and take everything up to a blank for the device
	    and then I locate the first space in this line and after that comes the mount-point
	   */
	     

	   k = strstr( tmp_bufferin, " " );

	   /*traverse the gap*/

	   if( isgraph( (int) *(k) ))
	     k++;
	   while(!(isgraph( (int) *(k) ))) 
	     k++;

	   /*
	     extract the path the device has been mounted to
	   */
	   l=0;

	   while(isgraph( (int) *(k) ))
	     {
	       path[l] = *(k);
	       k++;
	       l++;
	     }
	   path[l] = '\0';

	 }
       memset( tmp_bufferin, 0, MAX_STRING * sizeof( char ) );
       l = 0; /*for the next run
		       we take the last entry in /etc/mtab since that has been 
		       mounted the last and we assume that this is the
		       dvd. 
		     */
     }
     fclose( tmp_streamin );

     if(dvd_count == 0)
       { /* no cd found? Then user should mount it */
	 fprintf(stderr, "[Error] There seems to be no cd/dvd mounted. Please do that..\n");
	 return -1;
       }
   }
 else
   {
     fprintf(stderr, "[Error] Could not read /etc/mtab!");
     fprintf( stderr, "[Error] error: %s\n", strerror( errno ) );
     return -1;
   }
#endif
 return dvd_count;
}



/******************* get the whole vob size via stat() ******************/

off_t get_vob_size( int title, char *provided_input_dir ) 
{
      char  path_to_vobs[278], 
            path_to_vobs1[278], 
            path_to_vobs2[278], 
            path_to_vobs3[278];
      char  temp[278], 
            temp1[3], 
            stat_path[278];
      int   subvob;
      FILE  *tmp_streamin1;
      struct stat buf;
      off_t  vob_size = 0;     

   
      /* first option: path/vts_xx_yy.vob */
      sprintf( path_to_vobs, "%s/vts_%.2d", provided_input_dir, title ); /*which title-vob */      
      /* second option: path/VTS_XX_YY.VOB */
      sprintf( path_to_vobs1, "%s/VTS_%.2d", provided_input_dir, title ); /*which title-vob */      
      /* third option: path/VIDEO_TS/VTS_XX_YY.VOB */
      sprintf( path_to_vobs2, "%s/VIDEO_TS/VTS_%.2d", provided_input_dir, title ); /*which title-vob */      
      /* fourth option: path/video_ts/vts_xx_yy.vob */
      sprintf( path_to_vobs3, "%s/video_ts/vts_%.2d", provided_input_dir, title ); /*which title-vob */      
   
   
      /*
       * extract the size of the files on dvd using stat
       */
      sprintf( stat_path, "%s_1.vob", path_to_vobs );
      
      if( ( tmp_streamin1 = fopen( stat_path, "r" ) ) != NULL ) /*check if this path is correct*/
	{
	  fclose ( tmp_streamin1 );
	  subvob = 1;
	  while( !stat( stat_path, &buf ) )
	    {
	      /* adjust path for next subvob */
	       subvob++;
           sprintf( stat_path, "%s_%d.vob", path_to_vobs, subvob );	    
	       
	      vob_size += buf.st_size;
	    }	   
	   return ( off_t ) vob_size;
	}
      
      sprintf( stat_path, "%s_1.VOB", path_to_vobs1 );
      if( ( tmp_streamin1 = fopen( stat_path, "r" ) ) != NULL ) /*check if this path is correct */
	{
	  fclose ( tmp_streamin1 );
	  subvob = 1;
	  while( !stat( stat_path, &buf ) )
	    {
	      /* adjust path for next subvob */
	      subvob++;
          sprintf( stat_path, "%s_%d.VOB", path_to_vobs1, subvob );	    
	       
	      vob_size += buf.st_size;
	    }
   	   return ( off_t ) vob_size;
	}
      
      sprintf( stat_path, "%s_1.VOB", path_to_vobs2 );
      if( ( tmp_streamin1 = fopen( stat_path, "r" ) ) != NULL ) /*check if this path is correct */
	{
	  fclose ( tmp_streamin1 );
	  subvob = 1;
	  while( !stat( stat_path, &buf ) )
	    {
	      /* adjust path for next subvob */
	      subvob++;
          sprintf( stat_path, "%s_%d.VOB", path_to_vobs2, subvob );	     
	       
	      vob_size += buf.st_size;
	    }
   	   return ( off_t ) vob_size;
	}
   
      sprintf( stat_path, "%s_1.vob", path_to_vobs3 );
      if( ( tmp_streamin1 = fopen( stat_path, "r" ) ) != NULL ) /*check if this path is correct */
	{
	  fclose ( tmp_streamin1 );
	  subvob = 1;
	  while( !stat( stat_path, &buf ) )
	    {
	      /* adjust path for next subvob */
	      subvob++;
          sprintf( stat_path, "%s_%d.vob", path_to_vobs3, subvob );	      
	      vob_size += buf.st_size;
	    }
          return ( off_t ) vob_size; 
	}

      /*none of the above seemed to have caught it, so this is the error return */
      return ( off_t ) 0; /* think that (off_t) is not really needed here?
				  as it is defined as off_t and the function is
				  also defined as off_t */
}

