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
 *  along with vobcopy; if not,  see <http://www.gnu.org/licenses/>.
 */

#include "vobcopy.h"
#include <dvdread/ifo_read.h>

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
  strncpy( title, new_title, sizeof(title)-1 );
#else
  int  filehandle = 0;
  int  i = 0, last = 0;
  int  bytes_read;

  char tmp_buf[2048];

       
  /* open the device */
  if ( !(filehandle = open(device, O_RDONLY, 0)) )       
  {
      /* open failed */
      fprintf( stderr, _("[Error] Something went wrong while getting the dvd name  - please specify path as /cdrom or /dvd (mount point) or use -t\n"));
      fprintf( stderr, _("[Error] Opening of the device failed\n"));
      fprintf( stderr, _("[Error] Error: %s\n"), strerror( errno ) );
      return -1;
  }
  
  /* seek to title of first track, which is at (track_no * 32768) + 40 */
  if ( 32768 != lseek( filehandle, 32768, SEEK_SET ) )      
  {
      /* seek failed */
      close( filehandle );
      fprintf( stderr, _("[Error] Something went wrong while getting the dvd name - please specify path as /cdrom or /dvd (mount point) or use -t\n"));
      fprintf( stderr, _("[Error] Couldn't seek into the drive\n"));
      fprintf( stderr, _("[Error] Error: %s\n"), strerror( errno ) );
      return -1;
  }
  
  /* read title */
  if ( (bytes_read = read(filehandle, tmp_buf, 2048)) != 2048 )      
  {
      close(filehandle);
      fprintf( stderr, _("[Error] something wrong in dvd_name getting - please specify path as /cdrom or /dvd (mount point) or use -t\n") );
      fprintf( stderr, _("[Error] only read %d bytes instead of 2048\n"), bytes_read);
      fprintf( stderr, _("[Error] error: %s\n"), strerror( errno ) );
      return -1;
  }
  
  strncpy( title, &tmp_buf[40], 32 ); 
   
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
          fprintf( stderr, _("[Hint] The dvd has no name, will choose a nice one ;-), else use -t\n") );
          strcpy( title, "insert_name_here\0" );
      }
  else
      title[ last + 1 ] = '\0';

#endif

  for( i=0; i< strlen(title);i++ )
    {
      if( title[i] == ' ')
        title[i] = '_';
    }

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

#if ( defined( __sun ) )
  FILE  *mnttab_fp;
  char *mnt_special;
  int mntcheck;
  char *new_device;
#endif

  char  *pointer;
  char  *k;
  bool  mounted = FALSE;

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
       fprintf( stderr, _("[Error] Error while reading filesystem info") );
       return -1;
      }
#elif ( defined( __sun ) )

    struct mnttab mount_entry;
    int           mntcheck;
 
    if ( ( mnttab_fp = fopen( "/etc/mnttab", "r" ) ) == NULL )
      {
	fprintf( stderr, _(" [Error] Could not open mnttab for searching!\n") );
	fprintf( stderr, _(" [Error] error: %s\n"), strerror( errno ) );
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
		fprintf( stderr, _("[Error] %s doesn't look like a disk device to me"),
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
	    strncpy( device, new_device, sizeof(device)-1 );
	    free( mnt_special );
	    free( new_device );
	    mounted = TRUE;
	    break;
	  }
      }
    if ( mntcheck > 0 )
      {
         fprintf( stderr, _("[Error] Encountered error in reading mnttab file\n") );
         fprintf( stderr, _("[Error] error: %s\n"), strerror( errno ) );
         return -1;
      }
    else if ( mntcheck == -1 )
      {
         fprintf( stderr, _("[Error] Did not find mount %s in mnttab!\n"), path );
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
	      fprintf ( stderr, "[Info] Device %s mounted on %s\n", lmount_entry->mnt_fsname, lmount_entry->mnt_dir);
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
	fprintf( stderr, _("[Error] Could not read /etc/mtab!\n") );
	fprintf( stderr, _("[Error] error: %s\n"), strerror( errno ) );
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
		    fprintf( stderr, _("[Error] Weird, no /dev/ entry found in the line where iso9660 or udf gets mentioned in /etc/fstab\n") );
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
	              fprintf( stderr, _("[Hint] Hmm, the last char in the device path (%s) that gets mounted to %s is a number.\n"), device, path);
	          }
		device[l] = '\0';
	      }
	    memset( tmp_bufferin, 0, MAX_STRING * sizeof( char ) );
          }
          fclose( tmp_streamin );
	  if( !strstr( device, "/dev" ) )
	    {
	      fprintf( stderr, _("[Error] Could not find the provided path (%s), typo?\n"),path);
	      device[0] = '\0';
	      return -1;
	    }
      }
    else
      {
	fprintf( stderr, _("[Error] Could not read /etc/fstab!") );
	fprintf( stderr, _("[Error] error: %s\n"), strerror( errno ) );
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
	  fprintf( stderr, _("[Error] There seems to be no cd/dvd mounted. Please do that..\n"));
	  return -1;
        }
    }
  else
    {
      fprintf( stderr, _("[Error] An error occured while getting mounted file system information\n"));
      return -1;
    }

#elif ( defined( __sun ) )

    /* Completely rewritten search of a mounted DVD on Solaris, much
     cleaner. No more parsing of strings -- lb */

    /* the mnttab file */
    FILE         *mnttab_fp;
    /* The mount entry found */
    struct mnttab mountEntry;
    /* The mount search fields */
    struct mnttab udfsMount;
    /* Number of found entries */
    int           dvd_count = 0;
    /* Result of the mount search */
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
      fprintf( stderr, _("[Error] Could not open mnttab for searching!\n") );
      fprintf( stderr, _("[Error] error: %s\n"), strerror( errno ) );
      return -1;
	     }

    /* Get the matching mount points */
    /* If there is a match, handle it */
    while ((result = getmntany(mnttab_fp, &mountEntry, &udfsMount)) == 0) {
      dvd_count++;
	 }

    /* If the last result is not -1, there was a problem -- exit */
    if (result != -1)
      return -1;
   
    /* no cd found? Then user should mount it */
    if (dvd_count == 0) {
	 fprintf( stderr, _("[Error] There seems to be no cd/dvd mounted. Please do that..\n"));
	 return -1;
       }

    /* Copy the device name and mount points to the output variables */
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
	       fprintf( stderr, _("[Error] Weird, no /dev/ entry found in the line where iso9660, udf or cdrom gets mentioned in /etc/mtab\n") );
	       dvd_count--;
	       continue;
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
                    fprintf( stderr, _("[Hint] Hmm, the last char in the device path %s is a number.\n"), device );
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
                                          if (!strncmp(k, "\\040", 4)) 
                                          { /* replace escaped ASCII space ... */
                                                   path[l++] = ' '; /* ... with literal space */
                                                   k+=4;
                                          }
                                          else 
                                          {
                                                   path[l] = *(k);
                                                   k++;
                                                   l++;
                                          }
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
	 fprintf( stderr, _("[Error] There seems to be no cd/dvd mounted. Please do that..\n"));
	 return -1;
       }
   }
 else
   {
     fprintf( stderr, _("[Error] Could not read /etc/mtab!"));
     fprintf( stderr, _("[Error] error: %s\n"), strerror( errno ) );
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
      char  stat_path[278];
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

/*dvdtime2msec, converttime and get_longest_title are copy-paste'd from lsdvd*/

int dvdtime2msec(dvd_time_t *dt)
{
  double frames_per_s[4] = {-1.0, 25.00, -1.0, 29.97};
  double fps = frames_per_s[(dt->frame_u & 0xc0) >> 6];
  long   ms;
  ms  = (((dt->hour &   0xf0) >> 3) * 5 + (dt->hour   & 0x0f)) * 3600000;
  ms += (((dt->minute & 0xf0) >> 3) * 5 + (dt->minute & 0x0f)) * 60000;
  ms += (((dt->second & 0xf0) >> 3) * 5 + (dt->second & 0x0f)) * 1000;
  
  if(fps > 0)
    ms += ((dt->frame_u & 0x30) >> 3) * 5 + (dt->frame_u & 0x0f) * 1000.0 / fps;
  
  return ms;
}


void converttime(playback_time_t *pt, dvd_time_t *dt)
{
  double frames_per_s[4] = {-1.0, 25.00, -1.0, 29.97};
  double fps = frames_per_s[(dt->frame_u & 0xc0) >> 6];
  
  pt->usec = pt->usec + ((dt->frame_u & 0x30) >> 3) * 5 + (dt->frame_u & 0x0f) * 1000.0 / fps;
  pt->second = pt->second + ((dt->second & 0xf0) >> 3) * 5 + (dt->second & 0x0f);
  pt->minute = pt->minute + ((dt->minute & 0xf0) >> 3) * 5 + (dt->minute & 0x0f);
  pt->hour = pt->hour + ((dt->hour &   0xf0) >> 3) * 5 + (dt->hour   & 0x0f);
  
  if ( pt->usec >= 1000 ) { pt->usec -= 1000; pt->second++; }
  if ( pt->second >= 60 ) { pt->second -= 60; pt->minute++; }
  if ( pt->minute > 59 ) { pt->minute -= 60; pt->hour++; }
}


int get_longest_title( dvd_reader_t *dvd )
{
/*   dvd_reader_t *dvd; */
  ifo_handle_t *ifo_zero, **ifo; 
  pgcit_t *vts_pgcit;
  vtsi_mat_t *vtsi_mat;
  vmgi_mat_t *vmgi_mat;
  video_attr_t *video_attr;
  pgc_t *pgc;
  int i, j, titles, vts_ttn, title_set_nr;
  int max_length = 0, max_track = 0;
  struct dvd_info dvd_info;


  ifo_zero = ifoOpen(dvd, 0);
  if ( !ifo_zero ) {
    fprintf( stderr, "Can't open main ifo!\n");
    return 3;
  }

  ifo = (ifo_handle_t **)malloc((ifo_zero->vts_atrt->nr_of_vtss + 1) * sizeof(ifo_handle_t *));

  for (i=1; i <= ifo_zero->vts_atrt->nr_of_vtss; i++) {
    ifo[i] = ifoOpen(dvd, i);
    if ( !ifo[i] ) {
      fprintf( stderr, "Can't open ifo %d!\n", i);
      return 4;
    }
  }


  titles = ifo_zero->tt_srpt->nr_of_srpts;

  vmgi_mat = ifo_zero->vmgi_mat;

  
/*   dvd_info.discinfo.device = dvd_device; */
/*   dvd_info.discinfo.disc_title = has_title ? "unknown" : title; */
  dvd_info.discinfo.vmg_id =  vmgi_mat->vmg_identifier;
  dvd_info.discinfo.provider_id = vmgi_mat->provider_identifier;
  
  dvd_info.title_count = titles;
  dvd_info.titles = calloc(titles, sizeof(*dvd_info.titles));

  for (j=0; j < titles; j++)
    {

/*        GENERAL */
      if (ifo[ifo_zero->tt_srpt->title[j].title_set_nr]->vtsi_mat) {

	vtsi_mat   = ifo[ifo_zero->tt_srpt->title[j].title_set_nr]->vtsi_mat;
	vts_pgcit  = ifo[ifo_zero->tt_srpt->title[j].title_set_nr]->vts_pgcit;
	video_attr = &vtsi_mat->vts_video_attr;
	vts_ttn = ifo_zero->tt_srpt->title[j].vts_ttn;
	vmgi_mat = ifo_zero->vmgi_mat;
	title_set_nr = ifo_zero->tt_srpt->title[j].title_set_nr;
	pgc = vts_pgcit->pgci_srp[ifo[title_set_nr]->vts_ptt_srpt->title[vts_ttn - 1].ptt[0].pgcn - 1].pgc;

	dvd_info.titles[j].general.length = dvdtime2msec(&pgc->playback_time)/1000.0;
	converttime(&dvd_info.titles[j].general.playback_time, &pgc->playback_time);
	dvd_info.titles[j].general.vts_id = vtsi_mat->vts_identifier;
				
	if (dvdtime2msec(&pgc->playback_time) > max_length) {
	  max_length = dvdtime2msec(&pgc->playback_time);
	  max_track = j+1;
	}
      }
    }
  return max_track;
}
