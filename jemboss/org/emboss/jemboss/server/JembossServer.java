/****************************************************************
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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Date;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Random;
import java.util.StringTokenizer;
import java.util.Vector;

import org.emboss.jemboss.JembossParams;
import org.emboss.jemboss.gui.SequenceList;
import org.emboss.jemboss.programs.RunEmbossApplication2;

/**
*
* Jemboss Non-authenticated Server for Apache Axis (SOAP)
* web services
*
*/
public class JembossServer
{

  /** file separator */
  private String fs = System.getProperty("file.separator");

//get paths to EMBOSS
  /** jemboss properties */
  JembossParams jp = new JembossParams();
  /** plplot path */
  String plplot = jp.getPlplot();
  /** emboss data directory */
  String embossData = jp.getEmbossData();
  /** emboss binary directory */
  String embossBin = jp.getEmbossBin();
  /** emboss path */
  String embossPath = jp.getEmbossPath();
  /** acd directory */
  String acdDirToParse = jp.getAcdDirToParse();

  String[] embossCommandA = null;
  
  /** user home directory */
  private String homeDirectory = System.getProperty("user.home") + fs;
  /** user name */
  private String username = System.getProperty("user.name") + fs;
  /** results directory */
  private String tmproot  = jp.getResultsHome();
  /** results directory */
  private File tmprootDir = new File(tmproot);

  String projectName = null;
  
  /** emboss run environment */
  private String[] envp_emboss = 
  {
    "PATH=" + embossPath + System.getProperty("path.separator")
            + embossBin,
    "PLPLOT_LIB=" + plplot,
// cygwin: embossdata application does not like 
//         EMBOSS_DATA specified - it fails to 
//         fetch files
//  "EMBOSS_DATA=" + embossData,
    "HOME=" + homeDirectory,
    "EMBOSSCYGROOT=" + JembossParams.getCygwinRoot()
//  ,"LD_LIBRARY_PATH=/usr/local/lib"
// FIX FOR SOME SUNOS OR USE embossEnvironment

  };

  /** emboss run environment */
  private String[] envp = jp.getEmbossEnvironmentArray(envp_emboss);
 

  public JembossServer(){}

  public JembossServer(String tmproot)
  {
    this.tmproot = tmproot;
    tmprootDir = new File(tmproot);
  }

  public String name()
  {
    return "The EMBOSS Application Suite";
  }

  public String version()
  {
    String embossCommand = new String(embossBin + "embossversion");
    RunEmbossApplication2 rea = new RunEmbossApplication2(embossCommand,
                                                           envp,null);
    try
    {
      Process p = rea.getProcess();
      p.waitFor();
    }
    catch(InterruptedException iexp){}

    return rea.getProcessStdout();
  }


  public String appversion()
  {
    String embossCommand = new String(embossBin + "embossversion");
    RunEmbossApplication2 rea = new RunEmbossApplication2(embossCommand,
                                                           envp,null);
    try
    {
      Process p = rea.getProcess();
      p.waitFor();
    }
    catch(InterruptedException iexp){}

    return rea.getProcessStdout();
  }


  public String about()
  {
    return "Jemboss is an interface to the EMBOSS suite of programs.";
  }

  public String helpurl()
  {
    return "http://www.uk.embnet.org/Software/EMBOSS/";
  }

  public String abouturl()
  {
    return "http://www.uk.embnet.org/Software/EMBOSS/overview.html";
  }

  public String docurl()
  {
     return "http://www.uk.embnet.org/Software/EMBOSS/general.html";
  }

  public Hashtable servicedesc()
  {
     Hashtable desc = new Hashtable();
     desc.put("name",name());
     desc.put("version",version());
     desc.put("appversion",appversion());
     desc.put("about",about());
     desc.put("helpurl",helpurl());
     desc.put("abouturl",abouturl());
     desc.put("docurl",docurl());
     return desc;
  }


  /**
  *
  * Retrieves the ACD file of an application.
  * @param appName	application name
  * @return 		ACD string
  *
  */
  public Vector show_acd(String appName)
  {

    Vector acd = new Vector();
    String acdText = new String("");
    String line;
    String acdToParse = new String(acdDirToParse + appName + ".acd");

    BufferedReader in = null;
    try
    {
      in = new BufferedReader(new FileReader(acdToParse));
      while((line = in.readLine()) != null )
      {
        if(!line.startsWith("#") && !line.equals(""))
        { 
          line = line.trim();
          line = line.replace('}',')');
          acdText = acdText.concat(line + "\n");
        }
      }
      in.close();
    }
    catch (IOException e)
    {
      System.out.println("Cannot open EMBOSS acd file " + acdToParse);
    }
    finally
    {
    	if(in!=null)
			try {
				in.close();
			} catch (IOException e) {}
    }

    acd.add("status");
    acd.add("0");
    acd.add("acd");
    acd.add(acdText);

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
    Vector wossOut = new Vector();
    String embossCommand = new String(embossBin + "wossname -colon -gui -auto");
    RunEmbossApplication2 rea = new RunEmbossApplication2(embossCommand,
                                                           envp,null);
    wossOut.add("status");
    wossOut.add("0");
    try
    {
      Process p = rea.getProcess();
      p.waitFor();
    }
    catch(InterruptedException iexp){}

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
    String command = embossBin.concat("tfm " + applName + 
                                       " -html -nomore");
    RunEmbossApplication2 rea = new RunEmbossApplication2(command,
                                                     envp,null);

    try
    {
      Process p = rea.getProcess();
      p.waitFor();
    }
    catch(InterruptedException iexp){}

    String helptext = rea.getProcessStdout();
    if(helptext.equals(""))
      helptext = "No help available for this application.";

    Vector vans = new Vector();
    vans.add("helptext");
    vans.add(helptext);

    return vans;
  }


  /**
  *
  * Uses the infoseq application to calculate sequence length.
  * @param fileContent  sequence filename or database entry
  * @param seqtype      sequence type (seqset/sequence)
  * @param userName
  * @return         sequence length
  *
  */
  public Vector call_ajax(String fileContent, String seqtype, String userName)
  {
    return call_ajax(fileContent,seqtype);
  }

  /**
  *
  * Uses the infoseq application to calculate sequence length.
  * @param fileContent  sequence filename or database entry
  * @param seqtype      sequence type (seqset/sequence)
  * @return         sequence length
  *
  */
  
  public Vector call_ajax(String fileContent, String seqtype)
  {
        Vector vans = new Vector();
        // TODO: better to have JembosParams has a method to return its instance
        int ajaxLength = SequenceList.getSeqAttr(fileContent,
                new JembossParams());
        vans.add("status");
        if (ajaxLength == -1)
            vans.add("1");
        else {
            vans.add("0");
            vans.add("length");
            vans.add(new Integer(ajaxLength));
        }
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
    Vector showdbOut = new Vector();
    String embossCommand = new String(embossBin + "showdb -auto");
    RunEmbossApplication2 rea = new RunEmbossApplication2(embossCommand,
                                                          envp,null);
    showdbOut.add("status");
    showdbOut.add("0");

    try
    {
      Process p = rea.getProcess();
      p.waitFor();
    }
    catch(InterruptedException iexp){}

    showdbOut.add("showdb");
    showdbOut.add(rea.getProcessStdout());
     
    // find available matrices
    String dataFile[] = (new File(embossData)).list(new FilenameFilter()
    {
      public boolean accept(File dir, String name)
      {
          if (name.startsWith("EPAM") || name.startsWith("EBLOSUM") ||
                  name.startsWith("EDNA"))
              return true;
          
          return false;
      };
    });
    
    String matrices ="";
    for(int i=0;i<dataFile.length;i++)
      matrices=matrices.concat(dataFile[i]+"\n");
    showdbOut.add("matrices");
    showdbOut.add(matrices);

    // find available codon usage tables
    
    dataFile = (new File(embossData+fs+"CODONS")).list(new FilenameFilter()
    {
      public boolean accept(File dir, String name)
      {
        File fileName = new File(dir, name);
        return !fileName.isDirectory();
      };
    });
    matrices ="";
    for(int i=0;i<dataFile.length;i++)
      matrices=matrices.concat(dataFile[i]+"\n");
    showdbOut.add("codons");
    showdbOut.add(matrices);
    showdbOut.add("doesJembossServerSupportPDFoutput");
    showdbOut.add(Boolean.toString(jp.getEmbossHavePDF()));

    return showdbOut;
  }




  /**
  *
  * Run an EMBOSS application
  * @param embossCommand        command line to run
  * @param options              unused
  * @param inFiles              input files and names
  * @param userName             user name
  * @return                     output files from application run
  *
  */
  public Vector run_prog(String embossCommand, String options,
                         Vector inFiles, String userName)
  {
    Hashtable hashInFiles = getHashtable(inFiles);
    tmproot = tmproot.concat(userName+fs);
    return run_prog(embossCommand,options,hashInFiles);
  }


  
  /**
  *
  * Run an EMBOSS application.
  * Construction of the command line string here is for information only,
  * jobs are actually executed using the string array representation
  * of the command line.
  * 
  * @param embossCommand        command line represented as a String array
  * @param options              "interactive" or "batch", i.e sync or async job
  * @param inFiles              input files with names
  * @param userName             user name
  * @return                     output files from application run
  *
  */

  public Vector run_prog_array(Vector cmdA, String options,
          Vector inFiles, String userName)
  {
      Hashtable hashInFiles = getHashtable(inFiles);
      String cmd ="";
      int i = 0;
      tmproot = tmproot.concat(userName+fs);
      
      if(cmdA.size()==0)
          throw new RuntimeException("empty command line array");
      
      // it looks we have a new instance of this class for each ws request
      // so it look safe to have embossCommandA defined as class attribute
      embossCommandA = (String[])cmdA.toArray(new String[cmdA.size()]);
      embossCommandA[0] = embossBin + cmdA.get(0);
      
      cmd = (String)cmdA.get(0);

      for (i=1; i<cmdA.size(); i++)
      {
          String s = (String)cmdA.get(i);
          cmd += " "+(s.indexOf(' ')==-1 ? s : "\""+s+"\"");
      }

      return run_prog(cmd,options, hashInFiles);
  }

  /**
  *
  * Run an EMBOSS application
  * @param embossCommand	command line to run
  * @param options		unused 
  * @param inFiles		input files
  * @return 			output files from application run
  *
  */
  private Vector run_prog(String embossCommand, String options, 
                                            Hashtable inFiles)
  {

    Vector result = new Vector();

    //disallow multiple command constructions
    if(embossCommand.indexOf(";") > -1) 
    {
       result.add("msg");
       result.add("ERROR: Disallowed command syntax "+embossCommand);
       result.add("status");
       result.add("1");
       return result;
    }

    Enumeration enumFiles = inFiles.keys();
    String appl   = embossCommand.substring(0,embossCommand.indexOf(" "));
    String rest   = embossCommand.substring(embossCommand.indexOf(" "));
    embossCommand = embossBin.concat(embossCommand);
    String msg = new String("");

    boolean ok;
    
    if(!tmprootDir.isDirectory())
    {
      ok = tmprootDir.mkdirs();
      if(ok)
        System.out.println("Created directory "+tmproot);
      else
        System.out.println("Failed to create directory "+tmproot);
    }

    String dat = new Date().toString();
    dat = dat.replace(':','_');

    assignProjectName(appl);
    
//get a unique project name 
    String project = new String(tmproot + projectName);

    File projectDir = new File(project);
    ok = projectDir.mkdirs();

//create description file
    File desc = new File(new String(project + fs + ".desc"));

    String ls = System.getProperty("line.separator");
    String descript = "";
    try
    {
      PrintWriter dout = new PrintWriter(new FileWriter(desc));

      descript = descript.concat("EMBOSS run details"+ls+ls+
                                 "Application: "+appl+ls+rest+
                                 "Started at "+dat+ls+ls+"Input files:");
      dout.println("EMBOSS run details" + ls + ls);
      dout.println("Application: " + appl + ls + rest);
      dout.println("Started at " + dat + ls + ls + "Input files:");
  
      while (enumFiles.hasMoreElements())
      {
        String thiskey = (String)enumFiles.nextElement().toString();
        File f = new File(project + fs + thiskey);
        descript = descript.concat(project+fs+thiskey);
        dout.println(project + fs + thiskey);

        // Biocceleration: definition of "out" taken outside of try{},
        // added finally{}
        FileOutputStream out = null;
        try
        {
          f.createNewFile();
          out = new FileOutputStream(f);
          out.write((byte []) inFiles.get(thiskey));
          out.close();
        }
        catch(IOException ioe)
        {
          msg = new String("Error making input file");
        } finally {
            if (out != null) {
                out.close();
            }
        }

      }
      
      dout.close();
    }
    catch (IOException ioe) {} 

    RunEmbossApplication2 rea;
    if (embossCommandA != null){    	    	
    	rea = new RunEmbossApplication2(embossCommandA,
                envp,new File(project));
    } else {
    	rea = new RunEmbossApplication2(embossCommand, envp,new File(project));
    }
    
    result.add("cmd");
    result.add(appl + " " + rest);
    result.add("msg");
    result.add(msg);
    result.add("status");
    result.add(rea.getStatus());

    if(options.toLowerCase().indexOf("interactive") > -1)
    {
    	String stdout = "";
        String stderr = "";
        try
        {
          if (rea.getProcess() != null){
          rea.getProcess().waitFor();
          stdout = rea.getProcessStdout();
          stderr = rea.getProcessStderr();
          }
          else
          {
              stderr = rea.getInitialIOError();
          }
        }
        catch(InterruptedException iexp){}    
        if(!stdout.equals(""))
        {
          result.add("stdout");
          result.add(stdout);
        }
        
        if(!stderr.equals(""))
        {
          result.add("msg");
          result.add(stderr);
          createStderrFile(project, stderr);
        }
        createFinishedFile(project);
    }
    else      //batch or background
    {
      JembossThread jt = new JembossThread(rea,project);
      jt.start();

      result.add("job_submitted");
      result.add("Job " + projectDir.getName() + "submitted.");
      result.add("jobid");
      result.add(projectDir.getName());
      result.add("description");
      result.add(descript+ls+"Application pending"+ls);
    }

//get the output files
    loadFilesContent(projectDir,project,result,inFiles);
    loadPNGContent(projectDir,project,result);

    return result;
  }


  
  
  public String assignProjectName(String appl)
  {
      if (projectName==null)
      {
          Random rnd = new Random();
          String dat = new Date().toString();
          dat = dat.replace(':','_');

          //get a unique project name 
          projectName = new String(appl + "_" +
                  dat.replace(' ','_') + "_" + rnd.nextInt(99999));
      }
      return projectName;
  }
  
  
  
  
  public String getProjectName()
  {
      return projectName;
  }  
  
  
  
  
  /**
  *
  * Creates a file named "finished" in the project directory,
  * that contains a time stamp.
  * @param project	project directory name
  *
  */ 
  private void createFinishedFile(String project)
  {
    File finished = new File(new String(project + fs + ".finished"));
    // Biocceleration: definition of "fout" taken ouside of try{},
    // added finally{}
    PrintWriter fout = null;
    try
    {
      fout = new PrintWriter(new FileWriter(finished));
      fout.println((new Date()).toString());
      fout.close();
    }
    catch (IOException ioe) {}
    finally {
        if (fout != null) fout.close();
    }
  }




  /**
  *
  * Creates a file named "stderrfile" in the project directory
  *
  */
  
  private void createStderrFile(String project, String stderr)
  {
    File finished = new File(project + fs + "stderrfile");

    try
    {
      PrintWriter fout = new PrintWriter(new FileWriter(finished));
      fout.println(stderr);
      fout.close();
    }
    catch (IOException ioe) {}
  }





  /**
  *
  * Returns the results for a saved project.
  * @param project 	project directory name
  * @param cl		unused if showing all results otherwise
  *			this is the name of the file to display
  * @param userName	username
  * @return 		saved results files
  *
  */
  public Vector show_saved_results(String project, String cl,
                                   String userName)
  {
    Vector ssr = new Vector();
    tmproot = tmproot.concat(userName+fs); 
    ssr = show_saved_results(project,cl);
    return ssr;
  }

  /**
  *
  * Returns the results for a saved project.
  * @param project      project directory name
  * @param cl           unused if showing all results otherwise
  *                     this is the name of the file to display
  * @return             saved results files
  *
  */
  public Vector show_saved_results(String project, String cl)
  {
    Vector ssr = new Vector();
   
    project = tmproot.concat(project);
    File projectDir = new File(project);

    if(cl.equals(""))
    {
      loadFilesContent(projectDir,project,ssr,null);
      loadPNGContent(projectDir,project,ssr);
    }
    else
    {
      // Biocceleration: definition of "in" taken outside of try{},
      // added finally{}
      BufferedReader in = null;
      try
      {
        String fc = "";
        String line;
        File fn = new File(project + fs + cl);

        if(fn.exists())
        {
          in = new BufferedReader(new FileReader(project + fs + cl));
          while((line = in.readLine()) != null)
            fc = fc.concat(line + "\n");
        }
  
        ssr.add(cl);
        ssr.add(fc);
      }
      catch (IOException ioe){}
      finally {
          try {
            if (in != null) in.close();
          } catch (Exception e) {}
      }
    }

    ssr.add("status");
    ssr.add("0");

    ssr.add("msg");
    ssr.add("OK");

    return ssr;
  }


  /**
  *
  * Save a file to a project directory on the server.
  * @param project      project directory to save file in
  * @param filename     filename
  * @param notes        text to write to file
  * @param userName     username
  * @return             message
  *
  */
  public Vector save_project_file(String project, String filename,
                                  String notes, String userName)
  {
    return save_project_file(userName+ fs +project, filename, notes);
  }


  /**
  *
  * Save a file to a project directory on the server.
  * @param project	project directory to save file in
  * @param filename	filename
  * @param notes	text to write to file
  * @param userName	username
  * @return 		message
  *
  */
  public Vector save_project_file(String project, String filename,
                                  String notes)
  {
    Vector v = new Vector();
    String msg = "OK";

    String fn = tmproot + fs + 
                     project + fs + filename;

    File f = new File(fn);
    // Biocceleration: definition of "out" taken outside of try{},
    // added finally{}
    FileOutputStream out = null;
    try
    {
      if(!f.exists())
        f.createNewFile();
      out = new FileOutputStream(f);
      out.write(notes.getBytes());
      out.close();
    }
    catch(IOException ioe)
    {
      msg = new String("Error making input file");
    } finally {
        try {
            if (out != null) out.close();
        } catch (Exception e) {}
    }

    v.add("status");
    v.add("0");
    v.add("msg");
    v.add(msg);

    return v;
  }


  /**
  *  
  * Deletes a projects saved results.
  * @param project	project directory name
  * @param cl		unused
  * @param userName     username
  * @return 		message
  *
  */
  public Vector delete_saved_results(String project, String cl,
                                     String userName)
  {
    Vector dsr = null;
    tmproot = tmproot.concat(userName+fs);
    dsr = delete_saved_results(project,cl);
    return dsr;
  }

  /**
  * 
  * Deletes a projects saved results.
  * @param project      project directory name
  * @param cl           unused
  * @return             message
  *
  */
  public Vector delete_saved_results(String project, String cl)
  {
    Vector dsr = new Vector();
    boolean success = true;

    StringTokenizer st = new StringTokenizer(project,"\n");
    while(st.hasMoreTokens()) 
    {
      String proj = tmproot.concat(fs+st.nextToken());
    
      File projectDir = new File(proj);
      File resFiles[] = projectDir.listFiles();
      for(int i=0;i<resFiles.length;i++)
        success &= resFiles[i].delete();
      
      success &= projectDir.delete();
    }

    dsr.add("status");
    dsr.add(success ? "0":"1");
    dsr.add("msg");
    
    if(success)
    	dsr.add("Results deleted successfully.");
    else
    	dsr.add("Problems while deleting saved results.");
    
    return dsr;
  }


  /**
  *
  * List of the saved results on the server.
  * @param userName	username
  * @return 		list of the saved results.
  *
  */
  public Vector list_saved_results(String userName)
  {
    Vector lsr = new Vector();
    tmproot = tmproot.concat(userName+fs);
    tmprootDir = new File(tmproot);
    lsr = list_saved_results();
    return lsr;
  }

  /**
  *
  * List of the saved results on the server.
  * @return             list of the saved results.
  *
  */
  public Vector list_saved_results()
  {
    Vector lsr = new Vector();
   
    lsr.add("status");
    lsr.add("0");
    lsr.add("msg");
    lsr.add("OK");


    String resFiles[] = tmprootDir.list(new FilenameFilter()
    {
      public boolean accept(File cwd, String name)
      {
        return !name.startsWith(".");
      };
    });
    
    String list = "";
    for(int i=0;i<resFiles.length;i++)
    {
      String line = new String("");
      String fc = new String("");
     
      // Biocceleration: definition of "in" taken outside of try{},
      // added finally{}
      BufferedReader in = null;
      try
      {
        in = new BufferedReader(new FileReader(tmproot + fs +
                                               resFiles[i] + fs + ".desc"));
        while((line = in.readLine()) != null)
          fc = fc.concat(line + "\n");
        lsr.add(resFiles[i]);
        lsr.add(fc);
        list = list.concat(resFiles[i] + "\n");
      }
      catch (IOException ioe)
      {
        System.out.println("IOException in list_saved_results " +tmproot + fs +
                                               resFiles[i] + fs + ".desc");
      }
      finally {
          try {
            if (in != null) in.close();
          } catch (Exception e) {}
      }
    }

    lsr.add("list");
    lsr.add(list);

    return lsr;
  
  }


  /**
  *
  * Reads in files from EMBOSS output
  * @param projectDir   project directory
  * @param project      project name
  * @param result       results
  * @param inFiles      input files
  * @return             result
  *
  */
  private void loadFilesContent(File projectDir, String project, 
                                  Vector result, Hashtable inFiles)
  {
	  String outFiles[] = projectDir.list(new FilenameFilter()
	  {
		  public boolean accept(File cwd, String name)
		  {
			  return (!name.startsWith(".") && !name.endsWith(".png"));
		  };
	  });

	  if (outFiles == null)
		  return;

	  for(int i=0;i<outFiles.length;i++)
	  {
		  String line = new String("");
		  byte[] fc = null; // file content
		  String key = new String(outFiles[i]);

		  if(inFiles != null && inFiles.containsKey(key)) // leave out input files
			  continue;

		  File outfile = new File(project + fs + outFiles[i]);
		  fc = readByteFile(outfile.getAbsolutePath());

		  if(fc != null)
		  {
			  result.add(key);
			  result.add(fc);
		  }
	  }
	  return;
  }

  
  /**
  *
  * Reads in png files from EMBOSS output
  * @param projectDir   project directory
  * @param project      project name
  * @param result       results
  * @return             result
  *
  */
  private void loadPNGContent(File projectDir, String project, Vector result)
  {

    String pngFiles[] = projectDir.list(new FilenameFilter()
    {
      public boolean accept(File cwd, String name)
      {
        if( name.endsWith(".png") ||
            name.endsWith(".dat") )
          return true;
        return false;
      };
    });

    for(int i=0;i<pngFiles.length;i++)
    {
      String key = new String(pngFiles[i]);
      byte data[] = readByteFile(project + fs + pngFiles[i]);
      if(data != null)
      {
        result.add(key);
        result.add(data);
      }
    }
    return;
  }

  /**
  *
  * Read a file into a byte array.
  * @param filename	file name
  * @return 		byte[] contents of file
  *
  */
  protected static byte[] readByteFile(String filename)
  {

    File fn = new File(filename);
    byte[] b = null;
    // Biocceleration: definition of "fi" taken outside of try{},
    // added finally{}
    FileInputStream fi = null;
    try
    {
      long s = fn.length();
      if(s == 0)
        return b;
      b = new byte[(int)s];
      fi = new FileInputStream(fn);
      fi.read(b);
    }
    catch (IOException ioe)
    {
      System.out.println("Cannot read file: " + filename);
    } finally {
        try {
            if (fi != null) fi.close();
        } catch (Exception e) {}
    }
    return b;

  }


  /**
  *
  * Used to provide information on the batch/background
  * processes.
  * @param prog         program
  * @param opt          options
  * @param resToQuery   results to query
  * @param userName     user name
  *
  */
  public Vector update_result_status(String prog, String opt,
                        Vector resToQuery,String userName)
  {
    return update_result_status(prog,opt,getHashtable(resToQuery),
                                userName);
  }

  /**
  *
  * Used to provide information on the batch/background
  * processes.
  * @param prog         program
  * @param opt          options
  * @param resToQuery   results to query
  * @param userName     username
  *
  */
  public Vector update_result_status(String prog, String opt,
                        Hashtable resToQuery,String userName)
  {
    tmproot = tmproot.concat(userName+fs);
    return update_result_status(prog, opt, resToQuery);
  }

  /**
  *
  * Used to provide information on the batch/background
  * processes.
  * @param prog         program
  * @param opt          options
  * @param resToQuery   results to query
  *
  */
  public Vector update_result_status(String prog, String opt, 
                                     Hashtable resToQuery)
  {

    Vector vans = new Vector();
    Enumeration enumRes = resToQuery.keys();
    while (enumRes.hasMoreElements())
    {
      String thiskey = (String)enumRes.nextElement().toString();
      File f = new File(tmproot+fs+thiskey+fs+".finished");
      if(f.exists())
      {
        vans.add(thiskey);
        vans.add("complete");
        String fc = "";
        // Biocceleration: definition of "in" taken outside of try{}
        // added finally{}
        BufferedReader in = null;
        try
        {
          String line;
          in = new BufferedReader(new FileReader(tmproot+fs+thiskey+fs+".desc"));
          while((line = in.readLine()) != null)
            fc = fc.concat(line + "\n");
        }
        catch (IOException ioe)
        {
          fc = "Error in reading information file";
        } finally {
            try {
                if (in != null) in.close();
            } catch (Exception e) {}                
        }
        vans.add(thiskey+"-description");
        vans.add(fc);
      }
      else
      {
        vans.add(thiskey);
        vans.add("pending");
      }
    }

    return vans;
  }

  /**
  *
  * Convert contents from a Vector to a Hashtable
  * @param v    Vector
  *
  */
  private Hashtable getHashtable(Vector v)
  {
    Hashtable h = new Hashtable();
    for(Enumeration e = v.elements() ; e.hasMoreElements() ;)
    {
      String s = (String)e.nextElement();
      h.put(s,e.nextElement());
    }
    return h;
  }


  public final Object clone() throws java.lang.CloneNotSupportedException 
  {
    throw new java.lang.CloneNotSupportedException();
  }

}

