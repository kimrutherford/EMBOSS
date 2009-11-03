/*
*      
*  This program is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License
*  as published by the Free Software Foundation; either version 2
*  of the License, or (at your option) any later version.
* 
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
* 
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*  @author: Copyright (C) Tim Carver
* 
***************************************************************/

package org.emboss.jemboss.server;

import org.emboss.jemboss.JembossParams;
import org.emboss.jemboss.programs.RunEmbossApplication2;
import org.emboss.jemboss.parser.Ajax;

import java.io.*;
import java.util.*;

/**
*
* Jemboss Authenticated Server for Apache Axis (SOAP)
* web services
*
*/
public class JembossAuthServer
{

  /** SOAP results directory */
  private String tmproot = new String("/tmp/SOAP/emboss/");
  /** Jemboss log file       */
  private final String logFile = new String(tmproot+"/jemboss.log");
  /** Jemboss error log file */
  private final String errorLog = new String(tmproot+"/jemboss_error.log");
  /** file separator */
  private final String fs = new String(System.getProperty("file.separator"));
  /** path separator */
  private final String ps = new String(System.getProperty("path.separator"));
  /** line seperator */
  private final String ls = System.getProperty("line.separator");

//get paths to EMBOSS
  /** jemboss properties */
  final JembossParams jp  = new JembossParams();
  /** plplot path */
  final String plplot     = jp.getPlplot();
  /** emboss data path */
  final String embossData = jp.getEmbossData();
  /** emboss binary path */
  final String embossBin  = jp.getEmbossBin();
  /** path environment variable */
  final String embossPath = embossBin + ps + jp.getEmbossPath();

  /** emboss run environment */
  final private String[] env = 
  {
    "PATH=" + embossPath,
    "PLPLOT_LIB=" + plplot,
    "EMBOSS_DATA=" + embossData
//  ,"LD_LIBRARY_PATH=/usr/local/lib"
// FIX FOR SOME SUNOS
  };
 
  /** emboss run environment as a string */ 
  private final String environ = "PATH=" + embossPath+ " "+
                           "PLPLOT_LIB=" + plplot +" "+
                           "EMBOSS_DATA=" + embossData +" "+
                           jp.getEmbossEnvironment();
// "LD_LIBRARY_PATH=/usr/local/lib"+" ";
// FIX FOR SOME SUNOS


  /**
  *
  * Retrieves the ACD file of an application.
  * @param appName 	application name
  * @return 		Vector of containing the ACD string
  *
  */
  public Vector show_acd(String appName)
  {
    Vector acd = new Vector(4);
    StringBuffer acdText = new StringBuffer();
    final String acdToParse = new String(jp.getAcdDirToParse() + appName + ".acd");

    try
    {
      String line;
      BufferedReader in = new BufferedReader(new FileReader(acdToParse));
      while((line = in.readLine()) != null )
      {
        if(!line.startsWith("#") && !line.equals(""))
        { 
          line = line.trim();
          line = line.replace('}',')');
          acdText.append(line + "\n");
        }
      }
    }
    catch (IOException e)
    {
      appendToLogFile("Cannot open EMBOSS acd file "+acdToParse,errorLog);
    }

    acd.add("status");
    acd.add("0");
    acd.add("acd");
    acd.add(acdText.toString());

    return acd;
  }


  /**
  *
  * Returns the output of the EMBOSS utility wossname
  * @return 	wossname output
  *
  */
  public Vector getWossname()
  {
    final String[] envp = jp.getEmbossEnvironmentArray(env);
    Vector wossOut = new Vector(4);
    final String embossCommand = new String(embossBin + 
                   "wossname -colon -gui -auto");
 
    RunEmbossApplication2 rea = new RunEmbossApplication2(embossCommand,
                                                           envp,null);
    rea.waitFor();
    wossOut.add("status");
    wossOut.add(rea.getStatus());
    wossOut.add("wossname");
    wossOut.add(rea.getProcessStdout());

    return wossOut;
  }


  /**
  *
  * Returns the help for an application as given by 'tfm'
  * @param applName	application name
  * @return 		help 
  *
  */
  public Vector show_help(String applName)
  {
    final String[] envp = jp.getEmbossEnvironmentArray(env);
    final String command = embossBin.concat("tfm " + applName + " -html -nomore");
    RunEmbossApplication2 rea = new RunEmbossApplication2(command,
                                                       envp,null);
    rea.waitFor();
    String helptext = rea.getProcessStdout();
 
    if(helptext.equals(""))
      helptext = "No help available for this application.";

    Vector vans = new Vector(2);
    vans.add("helptext");
    vans.add(helptext);

    return vans;
  }


  /**
  *
  * Uses JNI to calculate sequence attributes using EMBOSS library call. 
  * @param fileContent	sequence filename or database entry
  * @param seqtype	sequence type (seqset/sequence)
  * @param userName	username
  * @param passwd       passwd
  * @return 	sequence length, weight & type (protein/nucleotide)
  *
  */
  public Vector call_ajax(String fileContent, String seqtype,
                          String userName, byte[] passwd)
  {
    Vector vans = new Vector(8);
    Ajax aj = new Ajax();
    if(!verifyUser(aj,userName,passwd,vans))
    {
      vans.trimToSize();
      return vans;
    }

    boolean afile = false;
    boolean fexists = false;

    // local file exists?
    if(fileContent.startsWith(fs))
    {
      int ind = fileContent.lastIndexOf(fs);
      String fdir  = fileContent.substring(0,ind);
      String ffile = fileContent.substring(ind+1).trim(); 
      boolean ok = aj.listFiles(userName,passwd,environ,fdir);

      if(!ok)
        return returnError(aj,"listFiles error in call_ajax",userName);

      if(aj.getOutStd().indexOf(fs+ffile+"\n") > -1)
        fexists = true;
      else 
        return returnError(aj,"listFiles error in call_ajax",userName);
    }

    // create temporary file
    String fn = null;
    if( ((fileContent.indexOf(":") < 0) || 
         (fileContent.indexOf("\n") > 0) ) &&
         (!fexists) ) 
    {
      try
      {
        fn = tmproot+fs+userName+fs+".jembosstmp";
        boolean ok = aj.putFile(userName,passwd,environ,
                             fn,fileContent.getBytes());

        if(!ok)
          return returnError(aj,"putFile error in call_ajax",userName);

        afile = true;
      }
      catch (Exception ioe) 
      {
        String dat = new Date().toString();
        dat = dat.replace(':','_');

        appendToLogFile(userName+":: "+dat+
                        " Exception: call_ajax creating "+fn,
                         errorLog);
        vans.add("status");
        vans.add("1");
        vans.trimToSize();
        return vans;
      }
    }
    else
    {
      fn = fileContent;     //looks like db entry or local file name
    }

    if( fexists  || afile ||    //call ajax if sequence file
        fn.indexOf(":") > 0 )   //or db
    {
      boolean ok = false;
      try
      {
        if(seqtype.startsWith("seqset"))
          ok = aj.seqsetAttrib(userName,passwd,environ,fn);
        else
          ok = aj.seqAttrib(userName,passwd,environ,fn);

        if(!ok)
          return returnError(aj,"seqAttrib or seqsetAttrib error",userName);

      }
      catch (Exception e)
      {
        String dat = new Date().toString();
        dat = dat.replace(':','_');

        appendToLogFile(userName+":: "+dat+
                        " Exception: call_ajax status not ok",
                         errorLog);
        vans.add("status");
        vans.add("1");
        vans.trimToSize();
        return vans;
      }
    }

    if(afile)
      aj.delFile(userName,passwd,environ,fn);

    for(int i=0;i<passwd.length;i++)
      passwd[i] = '\0';

    vans.add("length");
    vans.add(new Integer(aj.length_soap));
    vans.add("protein");
    vans.add(new Boolean(aj.protein_soap));
    vans.add("weight");
    vans.add(new Float(aj.weight_soap));
    vans.add("status");
    vans.add("0");

    return vans;
  }


  /**
  *
  * Uses JNI to calculate sequence attributes using EMBOSS library call. 
  * @param fileContent  sequence filename or database entry
  * @param seqtype      sequence type (seqset/sequence)
  * @return 		sequence length, weight & type (protein/nucleotide)
  *
  */
  public Vector call_ajax(String fileContent, String seqtype)
  {
    boolean afile = false;
    String fn = null;
    File tf = null;

    Vector vans = new Vector(8);

    // create temporary file
    if( ((fileContent.indexOf(":") < 0) ||
         (fileContent.indexOf("\n") > 0) ) &&
       !((new File(fileContent)).exists()) )
    {
      afile = true;
      try
      {
        File tmprootDir = new File(tmproot);
        tf = File.createTempFile("tmp",".jembosstmp", tmprootDir);
        PrintWriter out = new PrintWriter(new FileWriter(tf));
        out.println(fileContent);
        out.close();

        fn = new String(tf.getCanonicalPath());
      }
      catch (IOException ioe)
      {
        appendToLogFile("IOException: call_ajax creating tmp.jembosstmp",
                         errorLog);
        vans.add("status");
        vans.add("1");
        vans.trimToSize();
        return vans;
      }
    }
    else
    {
      fn = fileContent;     //looks like db entry or local file name
    }


    boolean ok = false;
    Ajax aj = null;

    if( ((new File(fn)).exists()) ||    //call ajax if sequence file
         (fn.indexOf(":") > 0) )        //or db
    {
      try
      {
        aj = new Ajax();
        if(seqtype.startsWith("seqset"))
          ok = aj.seqsetType(fn);
        else
          ok = aj.seqType(fn);
      }
      catch (Exception e)
      {
         appendToLogFile("Exception: call_ajax status not ok",
                         errorLog);
        vans.add("status");
        vans.add("1");
        vans.trimToSize();
        return vans;
      }
    }

    if(ok)
    {
//    System.out.println("STATUS OK");
      vans.add("length");
      vans.add(new Integer(aj.length));
      vans.add("protein");
      vans.add(new Boolean(aj.protein));
      vans.add("weight");
      vans.add(new Float(aj.weight));
      vans.add("status");
      vans.add("0");
    }
    else
    {
      appendToLogFile("Error: call_ajax status not ok",
                         errorLog);
      vans.add("status");
      vans.add("1");
      vans.trimToSize();
    }

    if(afile)
      tf.delete();

    return vans;
  }


  /**
  *
  * Returns the databases held on the server
  * @return 	output from 'showdb'
  *
  */
  public Vector show_db()
  {
    final String[] envp = jp.getEmbossEnvironmentArray(env);
    Vector showdbOut = new Vector(8);
    final String embossCommand = new String(embossBin + "showdb -auto");

    RunEmbossApplication2 rea = new RunEmbossApplication2(embossCommand,
                                                          envp,null);
    rea.waitFor();

    showdbOut.add("status");
    showdbOut.add(rea.getStatus());
     
    if(!rea.getStatus().equals("0"))
    {
      showdbOut.trimToSize();
      return showdbOut;
    }
                        
    showdbOut.add("showdb");
    showdbOut.add(rea.getProcessStdout());

    // find available matrices
    String dataFile[] = (new File(embossData)).list(new FilenameFilter()
    {
      public boolean accept(File dir, String name)
      {
        File fileName = new File(dir, name);
        return !fileName.isDirectory();
      };
    });
    StringBuffer matrices = new StringBuffer();
    for(int i=0;i<dataFile.length;i++)
      matrices.append(dataFile[i]+"\n");
    showdbOut.add("matrices");
    showdbOut.add(matrices.toString());

    // find available codon usage tables
    
    dataFile = (new File(embossData+fs+"CODONS")).list(new FilenameFilter()
    {
      public boolean accept(File dir, String name)
      {
        File fileName = new File(dir, name);
        return !fileName.isDirectory();
      };
    });
    matrices = new StringBuffer();
    for(int i=0;i<dataFile.length;i++)
      matrices.append(dataFile[i]+"\n");
    showdbOut.add("codons");
    showdbOut.add(matrices.toString());

    return showdbOut;
  }


  /**
  *
  * Run an emboss application
  * @param embossCommand        command line to run
  * @param options              options
  * @param inFiles              input files
  * @param userName             userName
  * @param passwd               passwd
  * @return 			output files from application run
  *
  */
  public synchronized Vector run_prog(String embossCommand, String options,
                            Vector inFiles, String userName, byte[] passwd)
  {
    tmproot = tmproot.concat(userName+fs);
    Ajax aj = new Ajax();
    Vector result = new Vector(6,1);

    if(!verifyUser(aj,userName,passwd,result))
      return result;

    String dat = new Date().toString();
    dat = dat.replace(':','_');

    //disallow multiple command constructions
    if(embossCommand.indexOf(";") > -1) 
    {
      String warn = new String(userName+":: "+dat+
                               " ERROR: Disallowed command syntax "+
                               embossCommand);
      appendToLogFile(warn,errorLog);
      result.add("msg");
      result.add(warn);
      result.add("status");
      result.add("1");
      result.trimToSize();
      return result;
    }

    //trap anything that is trying to write to stdout
    int stdIndex = embossCommand.indexOf(" stdout ");
    if(stdIndex > -1)
    {
      String startCmd = embossCommand.substring(0,stdIndex+7);
      String endCmd = embossCommand.substring(stdIndex+8);
      embossCommand = startCmd.concat("file ");
      embossCommand = embossCommand.concat(endCmd);
    }

    boolean ok = false; 
// listing of users root directory
    try
    { 
      aj.setErrStd();
      ok = aj.listDirs(userName,passwd,environ,tmproot);
    }
    catch(Exception exp){}


// create the results directory structure for this user if necessary
    if(!ok)
    {
      try
      {
        aj.setErrStd();
        ok = aj.makeDir(userName,passwd,environ,tmproot);
      }
      catch(Exception exp){}

//    if(ok && aj.getErrStd().equals(""))
//    Linux fix - ignore stderr here
      if(ok)
        appendToLogFile(userName+":: "+dat+" Created directory "+tmproot,errorLog);
      else
      {
        String warnmsg = new String(userName+":: "+dat+" Failed to create dir "+tmproot+
                                    "\nSTDERR :"+ aj.getErrStd());
        appendToLogFile(warnmsg,errorLog);
        result.add("msg");
        result.add(warnmsg);
        result.add("status");
        result.add("1");
        result.trimToSize();
        return result;
      }
    }

//get a unique project name
    String appl = embossCommand.substring(0,embossCommand.indexOf(" "));
    Random rnd = new Random();
    String project = new String(tmproot + appl + "_" +
         dat.replace(' ','_') + "_" + rnd.nextInt(99999));

    try
    {
      aj.setErrStd();
      ok = aj.makeDir(userName,passwd,environ,project);
    }
    catch(Exception exp){}

    if(!ok) 
      return returnError(aj,"run_prog failed to create dir "+
                          project,userName);

//create description file & input files
    StringBuffer descript = new StringBuffer();
    descript.append("EMBOSS run details for ");
    descript.append(appl);
    descript.append(ls);
    descript.append(ls);
    descript.append("Parameters Used: ");
    descript.append(embossCommand);
    descript.append(ls);
    descript.append("Started: ");
    descript.append(dat);
    descript.append(ls);
    descript.append(ls);
    descript.append("Input files:");
    descript.append(ls);

//write input files to project directory
    Vector inFileNames = new Vector();
    for(Enumeration e = inFiles.elements() ; e.hasMoreElements() ;)
    {
      String thiskey = (String)e.nextElement();  // file name
      inFileNames.add(thiskey);

      descript.append(project);
      descript.append(fs);
      descript.append(thiskey);
      descript.append(ls);

      ok = false;
      try
      {
        aj.setErrStd();
        ok = aj.putFile(userName,passwd,environ,
                 new String(project+fs+thiskey),
                 (byte[])e.nextElement());
      }
      catch(Exception exp){}

      if(!ok)
        return returnError(aj,"Failed to make file "+
                           project+fs+thiskey,userName);
    }

//write decription file to project directory
    ok = false;
    try
    {
      aj.setErrStd();
      ok = aj.putFile(userName,passwd,environ,
           new String(project + fs + ".desc"),
           descript.toString().getBytes());
    }
    catch(Exception exp){}

    if(!ok)
      return returnError(aj,"Failed to make file "+
                         project+fs+".desc",userName);

    new AppendToLogFileThread(userName+":: "+dat+" "+options+
                              " "+embossCommand,logFile,true).start();

    result.add("cmd");
    result.add(embossCommand);
    result.add("status");
    result.add("0");

    File projectDir = new File(project);
    if(options.toLowerCase().indexOf("interactive") > -1)
    {
      ok=true;
      try
      {
        aj.setErrStd();
        ok = aj.forkEmboss(userName,passwd,environ,
                             embossCommand,project);
      }
      catch(Exception exp){} 
 
//include any stderr from EMBOSS
      result.add("msg");
      result.add(aj.getErrStd());

//add a finished file
      try
      {
        aj.setErrStd();
        ok = aj.putFile(userName,passwd,environ,
             new String(project+fs+".finished"),
            (new Date()).toString().getBytes());
      }
      catch(Exception exp){}

      if(!ok)
        return returnError(aj,"putFile error in run_prog",userName);

//get the output files
      result = loadFilesContent(aj,userName,passwd,
                      projectDir,project,result,inFileNames);

      for(int i=0;i<passwd.length;i++)
        passwd[i] = '\0';
    }
    else      //batch or background
    {

// COMMENT THIS LINE TO USE QUEUEING SOFTWARE
      boolean lforkB = aj.forkBatch(userName,passwd,environ,
                                    embossCommand,project);

// UNCOMMENT ONE OF THESE LINE TO USE QUEUEING SOFTWARE
//    runAsBatch(aj,userName,passwd,project,quoteMe(embossCommand));
//    runAsGNQSBatch(aj,userName,passwd,project,quoteMe(embossCommand));
//    runAsPBSBatch(aj,userName,passwd,project,quoteMe(embossCommand));
      result.add("msg");
      result.add("");
      result.add("job_submitted");
      result.add("Job " + projectDir.getName() + "submitted.");
      result.add("jobid");
      result.add(projectDir.getName());
      result.add("description");
      result.add(descript+ls+"Application pending"+ls);
    }

    return result;
  }


  /**
  *
  * Quote all tokens ready for shell scripts
  * @param s	text to quote
  * @return	quoted text
  *
  */
  private String quoteMe(String s)
  {
    StringBuffer qs = new StringBuffer();
    StringTokenizer st = new StringTokenizer(s.trim()," ");
    String tok;
    while (st.hasMoreTokens())
    {
      tok = st.nextToken().trim();
      if(!tok.equals(" "))
      {
        qs.append("\"");
        qs.append(tok);
        qs.append("\" ");
      }
    }
    return qs.toString();
  }

  /**
  *
  * Submit to a OpenPBS batch queue. This method creates a script for
  * submission to a batch queueing system.
  * @param aj			ajax/jni
  * @param userName		username
  * @param passwd		passwd
  * @param project		project directory
  * @param embossCommand	emboss command
  *
  */
  private void runAsPBSBatch(Ajax aj, String userName, byte[] passwd,
                              String project, String embossCommand)
  {
    StringBuffer scriptIt = new StringBuffer();
    scriptIt.append("#PBS -j oe\n");
    scriptIt.append("#PBS -S /bin/sh\n");
    scriptIt.append(environ.replace(' ','\n'));
    scriptIt.append("\nexport PATH\n");
    scriptIt.append("export PLPLOT_LIB\n");
    scriptIt.append("export EMBOSS_DATA\n");
    scriptIt.append("cd "+project+"\n"+embossCommand+"\n");
    scriptIt.append("date > "+project+"/.finished\n");

    String scriptFile = new String(project+fs+".scriptfile");
    boolean ok = false;
    try
    {
      ok = aj.putFile(userName,passwd,environ,scriptFile,
                      scriptIt.toString().getBytes());
    }
    catch(Exception exp){}

    if(!ok)
    {
      String dat = new Date().toString();
      dat = dat.replace(':','_');

      appendToLogFile(userName+":: "+dat+
                      " Failed to make file "+project+fs+".scriptfile\n"+
                      "STDERR "+aj.getErrStd()+"\n"+
                      "STDOUT "+aj.getOutStd(),errorLog);
    }

    boolean lfork=true;
    try
    {
      //EDIT batchCommand
      String batchCommand = "/hgmp/local/openpbs/bin/qsub "+scriptFile;
      lfork = aj.forkEmboss(userName,passwd,environ,
                            batchCommand,project);
    }
    catch(Exception exp){}

    if(!lfork || !aj.getErrStd().equals(""))
    {
      String dat = new Date().toString();
      dat = dat.replace(':','_');

      appendToLogFile(userName+":: "+dat+
                      " Fork batch process failed "+embossCommand,errorLog);
    }

    return;
  }

  /**
  *
  * Submit to a Generic NQS batch queue. This method creates a script for
  * submission to a batch queueing system.
  * @param aj                   ajax/jni
  * @param userName             username
  * @param passwd               passwd
  * @param project              project directory
  * @param embossCommand        emboss command
  *
  */
  private void runAsGNQSBatch(Ajax aj, String userName, byte[] passwd,
                              String project, String embossCommand)
  {
    String scriptIt = "#QSUB -q jemboss-queue\n";
    scriptIt = scriptIt.concat("#QSUB -s /bin/sh\n");
    scriptIt = scriptIt.concat("#QSUB -eo -o report.nqs\n");
    scriptIt = scriptIt.concat(environ.replace(' ','\n'));
    scriptIt = scriptIt.concat("\nexport PATH\n");
    scriptIt = scriptIt.concat("export PLPLOT_LIB\n");
    scriptIt = scriptIt.concat("export EMBOSS_DATA\n");
    scriptIt = scriptIt.concat("cd "+project+"\n"+embossCommand+"\n");
    scriptIt = scriptIt.concat("date > "+project+"/.finished\n");

    String scriptFile = new String(project+fs+".scriptfile");
    boolean ok = false;
    try
    {
      ok = aj.putFile(userName,passwd,environ,scriptFile,
                      scriptIt.getBytes());
    }
    catch(Exception exp){}

    if(!ok)
    {
      String dat = new Date().toString();
      dat = dat.replace(':','_');

      appendToLogFile(userName+":: "+dat+" Failed to make file "+project+fs+".scriptfile",errorLog);
      appendToLogFile(userName+":: "+dat+" STDERR "+aj.getErrStd(),errorLog);
      appendToLogFile(userName+":: "+dat+" STDOUT "+aj.getOutStd(),errorLog);
    }

    boolean lfork=true;
    try
    {
      //EDIT batchCommand
      String batchCommand = "/hgmp/local/nqs/bin/qsub "+scriptFile;
      lfork = aj.forkEmboss(userName,passwd,environ,
                            batchCommand,project);
    }
    catch(Exception exp){}

    if(!lfork || !aj.getErrStd().equals(""))
    {
      String dat = new Date().toString();
      dat = dat.replace(':','_');

      appendToLogFile(userName+":: "+dat+
                      " Fork batch process failed "+embossCommand,errorLog);
    }
    
    return;
  }

  /**
  *
  * Submit to a batch queue
  * @param aj                   ajax/jni
  * @param userName             username
  * @param passwd               passwd
  * @param project              project directory
  * @param embossCommand        emboss command
  *
  */
  private void runAsBatch(Ajax aj, String userName, byte[] passwd,
                    String project, String embossCommand)
  {
    String scriptIt = "#!/bin/sh\n";
    scriptIt = scriptIt.concat(environ.replace(' ','\n'));
    scriptIt = scriptIt.concat("\nexport PATH\n");
    scriptIt = scriptIt.concat("export PLPLOT_LIB\n");
    scriptIt = scriptIt.concat("export EMBOSS_DATA\n");
    scriptIt = scriptIt.concat("cd "+project+"\n"+embossCommand+"\n");
    scriptIt = scriptIt.concat("date > "+project+"/.finished\n");
    
    boolean ok = false;
    try
    {
      ok = aj.putFile(userName,passwd,environ,
               new String(project+fs+".scriptfile"),
               scriptIt.getBytes());
    }
    catch(Exception exp){}

    if(!ok)
    {
      String dat = new Date().toString();
      dat = dat.replace(':','_');
    
      appendToLogFile(userName+":: "+dat+" Failed to make file "+project+fs+".scriptfile",errorLog);
      appendToLogFile(userName+":: "+dat+" STDERR "+aj.getErrStd(),errorLog);
      appendToLogFile(userName+":: "+dat+" STDOUT "+aj.getOutStd(),errorLog);
    }
  
    boolean lfork=true;
    try
    {
      //EDIT batchCommand 
      String batchCommand = "/bin/batchQueue.sh " + project +
                            "/.scriptfile ";
                         
      lfork = aj.forkEmboss(userName,passwd,environ,
                            batchCommand,project);
    }
    catch(Exception exp){}

    if(!lfork || !aj.getErrStd().equals(""))
    {
      String dat = new Date().toString();
      dat = dat.replace(':','_');

      appendToLogFile(userName+":: "+dat+
                      " Fork batch process failed "+embossCommand,errorLog);
    }
    return;
  }

  /**
  *
  * Returns the results for a saved project.
  * @param project 	project directory name
  * @param cl		unused if showing all results 
  *			otherwise this is the name of 
  *    			the file to display
  * @param userName	username
  * @param passwd	passwd
  * @return 		saved results files
  *
  */
  public Vector show_saved_results(String project, String cl,
                              String userName, byte[] passwd)
  {

    Ajax aj = new Ajax();
    Vector ssr = new Vector();
    if(!verifyUser(aj,userName,passwd,ssr))
      return ssr;

    project = tmproot.concat(userName+fs+project);

    if(cl.equals(""))
    {
      File projectDir = new File(project);
      ssr = loadFilesContent(aj,userName,passwd,
                      projectDir,project,ssr,null);
    }
    else
    {
      byte fbuf[]=null;
      try
      {
        fbuf =  aj.getFile(userName,passwd,environ,
                                  project+fs+cl);
        ssr.add(cl);
        ssr.add(new String(fbuf));
      }
      catch(Exception exp){}
    }
 
    ssr.add("status");
    ssr.add("0");
    ssr.add("msg");
    ssr.add("OK");

    for(int i=0;i<passwd.length;i++)
      passwd[i] = '\0';

    return ssr;
  }

  /**
  *  
  * Save a file to a project directory on the server.
  * @return 	message
  *
  */
  public Vector save_project_file(String project, String filename, 
                    String notes, String userName, byte[] passwd)
  {
    Ajax aj = new Ajax();
    Vector v = new Vector(4);
    if(!verifyUser(aj,userName,passwd,v))
    {
      v.trimToSize();
      return v;
    }

    final String fn = tmproot + fs + userName+ fs + 
                     project + fs + filename;
    boolean ok = aj.putFile(userName,passwd,environ,
                            fn,notes.getBytes());

    v.add("status");
    v.add("0");
    v.add("msg");

    for(int i=0;i<passwd.length;i++)
      passwd[i] = '\0';

    if(ok)
      v.add("OK"); 
    else
      return returnError(aj,"Failed to save file "+fn,userName);
        
    return v;
  }

  /**
  *  
  * Deletes a projects saved results.
  * @param project	project directory name
  * @param cl		unused
  * @param userName	username
  * @param passwd	passwd
  * @return message
  *
  */
  public Vector delete_saved_results(String project, String cl,
                                String userName, byte[] passwd)
  {
    Vector dsr = new Vector(4);
    Ajax aj = new Ajax();
    if(!verifyUser(aj,userName,passwd,dsr))
    {
      dsr.trimToSize();
      return dsr;
    }

    tmproot = tmproot.concat(userName+fs);

    StringTokenizer st = new StringTokenizer(project,"\n");
    while(st.hasMoreTokens()) 
    {
      String proj = tmproot.concat(st.nextToken());
      boolean ok = aj.delDir(userName,passwd,environ,proj);
      if(!ok || !aj.getErrStd().equals(""))
        return returnError(aj,"Failed deletion of directory "+proj,userName);
    }

    dsr.add("status");
    dsr.add("0");
    dsr.add("msg");
    dsr.add("Results deleted successfully.");

    for(int i=0;i<passwd.length;i++)
      passwd[i] = '\0';

    return dsr;
  }


  /**
  *
  * List of the saved results on the server.
  * @param userName     username
  * @param passwd       passwd
  * @return 		list of the saved results.
  *
  */
  public Vector list_saved_results(String userName, byte[] passwd)
  {
    Ajax aj = new Ajax();
    Vector lsr = new Vector();
    if(!verifyUser(aj,userName,passwd,lsr))
      return lsr;

    tmproot = tmproot.concat(userName+fs);

    lsr.add("status");
    lsr.add("0");
    lsr.add("msg");
    lsr.add("OK");

    aj.setErrStd();
    boolean lsd = aj.listDirs(userName,passwd,environ,tmproot);
    
    String outStd = aj.getOutStd();

    int indStart = 0;
    int indEnd   = 0;
//  StringTokenizer stok = new StringTokenizer(outStd,"\n");
//  while(stok.hasMoreTokens())
    while((indEnd = outStd.indexOf("\n",indStart)) > -1) 
    {
//    String dirname = stok.nextToken();
      String dirname = outStd.substring(indStart,indEnd);
      indStart = indEnd+1;

      lsr.add(dirname);
      byte fbuf[] = aj.getFile(userName,passwd,environ,
                     tmproot + dirname + fs + ".desc");
      lsr.add(new String(fbuf));

      if(aj.getFileok()!=1)
      {
        String dat = new Date().toString();
        dat = dat.replace(':','_');

        appendToLogFile(userName+":: "+dat+" Calling getFile : "+tmproot +
                        dirname + fs + ".desc\n"+
                        "STDERR "+aj.getErrStd()+"\n"+
                        "STDOUT "+aj.getOutStd(),errorLog);
      }
    }
    
    lsr.add("list");
    lsr.add(outStd);

    for(int i=0;i<passwd.length;i++)
      passwd[i] = '\0';

    return lsr;
  }


  /**
  *
  * Appends a log entry to the log file
  * @param logEntry	entry to add to log file
  * @param logFileName 	log file name
  *
  */ 
  private void appendToLogFile(String logEntry, String logFileName)
  {
    BufferedWriter bw = null;
    try 
    {
      bw = new BufferedWriter(new FileWriter(logFileName, true));
      bw.write(logEntry);
      bw.newLine();
      bw.flush();
    } 
    catch (Exception ioe) 
    {
      System.out.println("Error writing to log file "+logFile);
      ioe.printStackTrace();
    } 
    finally                     // always close the file
    {                       
      if(bw != null) 
      try
      {
        bw.close();
      } 
      catch (IOException ioe2) {}
    }

  }


  /**
  *
  * Reads in files from EMBOSS output
  * @param aj		ajax/jni
  * @param userName	username
  * @param passwd	passwd
  * @param projectDir	project directory
  * @param project     	project name
  * @param result	results
  * @param inFiles	input files
  * @return		result
  *
  */
  private Vector loadFilesContent(Ajax aj, String userName, 
            byte[] passwd, File projectDir, String project,
            Vector result, Vector inFiles)
  {

    boolean ls = false;

    try
    {
      ls = aj.listFiles(userName,passwd,environ,project);
    }
    catch(Exception exp){}

    if(!ls)
    {
      String dat = new Date().toString();
      dat = dat.replace(':','_');

      appendToLogFile(userName+":: "+dat+
                      " Failed loadFilesContent\n"+
                      "STDERR "+aj.getErrStd()+"\n"+
                      "STDOUT "+aj.getOutStd(),errorLog);
    }

    StringTokenizer stok = new StringTokenizer(aj.getOutStd(),"\n");
    String fn =  null;
    while(stok.hasMoreTokens()) 
    {
      fn = stok.nextToken();
      if(inFiles == null || !inFiles.contains(fn))
      {
        byte fbuf[]=null;
        try
        {
          fbuf =  aj.getFile(userName,passwd,environ,
                                    project+fs+fn);
        }
        catch(Exception exp){}
        if(aj.getFileok()==1)
        {
          result.add(fn);
          result.add(fbuf);
        }
        else
        {
          String dat = new Date().toString();
          dat = dat.replace(':','_');

          appendToLogFile(userName+":: "+dat+
                          " Cannot getFile "+project+fs+fn,errorLog);
        }
      }
    }

    return result;
  }


  /**
  *
  * Used to provide information on the batch/background
  * processes.
  * @param prog         program
  * @param opt          options
  * @param resToQuery   results to query
  * @param userName     username
  * @param passwd       passwd
  *
  */
  public Vector update_result_status(String prog, String opt,
                        Vector resToQuery,String userName,
                        byte[] passwd)
  {
    Ajax aj = new Ajax();
    Vector vans = new Vector();
    if(!verifyUser(aj,userName,passwd,vans))
      return vans;

    tmproot = tmproot.concat(userName+fs);

    for(Enumeration e = resToQuery.elements() ; e.hasMoreElements() ;)
    {
      String thiskey = (String)e.nextElement();
      e.nextElement();

      try
      {
        aj.getFile(userName,passwd,environ,
                 tmproot+thiskey+fs+".finished");
      }
      catch(Exception ex){}

      if((aj.getErrStd().indexOf("stat error") == -1) &&
         (aj.getFileok()==1))
      {
        vans.add(thiskey);
        vans.add("complete");
        
        byte fbuf[] = aj.getFile(userName,passwd,environ,
                tmproot + fs + thiskey+ fs + ".desc");

        if(aj.getFileok()==1)
        {
          vans.add(thiskey+"-description");
          vans.add(new String(fbuf));
        }
      }
      else
      {
        vans.add(thiskey);
        vans.add("pending");
      }
    }

    for(int i=0;i<passwd.length;i++)
      passwd[i] = '\0';

    return vans;
  }


  /**
  *
  * Verify the username/passwd
  * @param aj		ajax/jni
  * @param userName 	username
  * @param passwd	password
  * @param res		results vector
  * @return		true if authenticated ok
  *
  */
  private boolean verifyUser(Ajax aj, String userName, 
                            byte[] passwd, Vector res)
  {

    if(userName == null || passwd == null)
    {
//    System.out.println("Failed Authorisation "+userName);
      res.add("msg");
      res.add("Failed Authorisation ");
      res.add("status");
      res.add("1");
      return false;
    }

    boolean ok = false;

    try
    {
      ok = aj.userAuth(userName,passwd,environ);
    }
    catch(Exception exp) 
    {
      ok = false;
    }

    if(!ok)
    {
      String dat = new Date().toString();
      dat = dat.replace(':','_');

      appendToLogFile(userName+":: "+dat+
                      " Failed Authorisation "+userName+"\n"+
                      "STDERR "+aj.getErrStd(),errorLog);
      res.add("msg");
      res.add("Failed Authorisation "+userName);
      res.add("status");
      res.add("1");
      return false;
    }

    return true;
  }

  /**
  *
  * Report the stderr and stdout to error logs
  * @param aj	ajax/jni
  * @param msg 	message
  * @return	vector containing the message 
  *
  */
  private Vector returnError(Ajax aj, String msg, String userName)
  {
    String dat = new Date().toString();
    dat = dat.replace(':','_');

    appendToLogFile(userName+":: "+dat+
                    " STDERR "+aj.getErrStd()+"\n"+
                    "STDOUT "+aj.getOutStd()+"\n"+
                    "MSG    "+msg,errorLog);

    Vector vans = new Vector(4);
    vans.add("msg");
    vans.add(aj.getErrStd());

    vans.add("status");
    vans.add("1");
    return vans;
  }

  public final Object clone() throws java.lang.CloneNotSupportedException
  {
    throw new java.lang.CloneNotSupportedException();
  }

}

