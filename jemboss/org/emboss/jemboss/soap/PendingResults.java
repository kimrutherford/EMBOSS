/********************************************************************
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public
*  License along with this library; if not, write to the
*  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
*  Boston, MA  02111-1307, USA.
*
*  @author: Copyright (C) Tim Carver
*
********************************************************************/

package org.emboss.jemboss.soap;

import org.emboss.jemboss.JembossParams;
import org.emboss.jemboss.gui.*;
import org.emboss.jemboss.programs.*;
import org.emboss.jemboss.server.JembossServer;

import java.util.*;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/**
*
* Record the batch job status (pending/completed)
*
*/
public class PendingResults 
{

  /** number of completed processes */
  private int completed_jobs = 0;
  /** number of peding processes */
  private int running_jobs = 0;
  /** jemboss properties */
  private JembossParams mysettings;
  /** pending results */
  private Vector pendingResults;
  /** job manager button */
  private JButton jobButton = null;
  /** job manager interactive/batch selection */
  private JComboBox jobComboBox = null;
  /** automatic job status updates */
  private boolean autoUpdates = false;
  private boolean withSoap = true;


  /**
  *
  * @param mysettings   jemboss properties
  * @param withSoap     false if we are not connecting to a server
  *
  */
  public PendingResults(JembossParams mysettings, final boolean withSoap)
  {
    this(mysettings);
    this.withSoap = withSoap;
  }

  /**
  *
  * @param mysettings	jemboss properties
  *
  */
  public PendingResults(JembossParams mysettings)
  {
    this.mysettings = mysettings;
    pendingResults = new Vector();
  }

 
  /**
  *
  * Standalone method to record start of new process.
  *
  */
  public void addRunningJob()
  {
    running_jobs++;
    if(jobButton != null)
      jobButton.setText(jobStatus());
  }

  /**
  *
  * Standalone method to record end of new process.
  *
  */
  public void deleteRunningJob()
  {
    running_jobs--;
    completed_jobs++; 
    if(jobButton != null)
      jobButton.setText(jobStatus());
  }


  /**
  *
  * Reset the pending and completed job status
  * 
  */
  public void resetCount() 
  {
    completed_jobs = 0;
    running_jobs = 0;
    pendingResults.removeAllElements();
  }


  /**
  *
  * Add a pending process to the list
  * @param res	jemboss process
  * 
  */
  public void addResult(JembossProcess res) 
  {
    pendingResults.add(res);
  }

  /**
  *
  * Remove a pending process from the list
  * @param res  jemboss process
  * 
  */
  public void removeResult(JembossProcess res)
  {
    pendingResults.remove(res);
  }

  /**
  *
  * Get the processes result
  * @param s  	name of the dataset
  * @return 	process object
  *
  */
  public JembossProcess getResult(String s)
  {
    for (int i=0 ; i < pendingResults.size(); ++i)
    {
      JembossProcess er = (JembossProcess)pendingResults.get(i);
      if(er.getJob().equals(s))
        return er;
    }
    return null;
  }

  /**
  *
  * Get the processes description
  * @return	processes description
  *
  */
  public Hashtable descriptionHash() 
  {
    Hashtable h = new Hashtable();
    for (int i=0 ; i < pendingResults.size(); ++i) 
    {
      JembossProcess er = (JembossProcess)pendingResults.get(i);
      String desc = er.getDescription();
      if(desc == null || desc.equals("")) 
        desc = " Pending";
      h.put(er.getJob(),desc);
    }
    return h;
  }

  /**
  *
  * Update the number of completed and pending
  * processes
  * 
  */
  public void updateJobStats() 
  {
    int ic = 0;
    int ir = 0;
    for (int i=0 ; i < pendingResults.size(); ++i) 
    {
      JembossProcess er = (JembossProcess)pendingResults.get(i);
      if(er.isCompleted())  
        ++ic; 
      if(er.isRunning())  
        ++ir; 
    }
    completed_jobs = ic;
    running_jobs = ir;
  }

  /**
  *
  * Report the status of completed and running processes.
  * @return	completed / pending numbers
  *
  */
  public String jobStatus() 
  {
    String sc =  new Integer(completed_jobs).toString();
    String sr =  new Integer(running_jobs).toString();
    String s;

    if(completed_jobs == 0) 
    {
      if(running_jobs == 0) 
        s = "Jobs: no pending jobs";
      else 
        s = "Jobs: " + sr + " running";
    }
    else
    {
      if (running_jobs == 0) 
        s = "Jobs: " + sc + " completed";
      else 
        s = "Jobs: " + sc + " completed / " + sr + " running";
    }
    return s;
  }

  /**
  *
  * Connect to the Jemboss server, and update the status of the jobs
  * in the list. If a statusPanel is active, updates the text on that.
  *
  */
  public void updateStatus() 
  {
    Hashtable resToQuery = new Hashtable();

    //initialize hash with project/jobid
    for (int i=0 ; i < pendingResults.size(); ++i) 
    {
      JembossProcess er = (JembossProcess)pendingResults.get(i);
      resToQuery.put(er.getJob(),er.getProject());
    }

    Vector params = new Vector();
    params.addElement("");
    params.addElement("");
    params.addElement(getVector(resToQuery));
    try 
    {
      final Hashtable hashStatus;

      if(withSoap)
      {
        PrivateRequest eq = new PrivateRequest(mysettings,
                          "update_result_status", params);
        hashStatus = eq.getHash();
      }
      else
      {
        JembossServer js = new JembossServer(mysettings.getResultsHome());
        Vector vecStatus = js.update_result_status("", "", resToQuery);
        hashStatus = org.emboss.jemboss.gui.form.BuildJembossForm.convert(vecStatus, false);
      }

      // update the results
      for(int i=0; i < pendingResults.size(); ++i) 
      {
        JembossProcess er = (JembossProcess)pendingResults.get(i);
        String jobid = er.getJob();
        String s = (String)hashStatus.get(jobid);
        if (mysettings.getDebug()) 
          System.out.println("PendingResults: "+jobid+" : "+s);

        if (s.equals("complete"))
        {
          er.complete();
          String sd = (String)hashStatus.get(jobid+"-description");
          if (!sd.equals("")) 
            er.setDescription(sd);
        }
      }
      updateJobStats();
      if (jobButton != null) 
        jobButton.setText(jobStatus());
      
    } 
    catch (JembossSoapException e)
    {
      //throw new JembossSoapException();
    }

  }

  /**
  *
  * Change a hashtable to a vector
  *
  */
  private Vector getVector(Hashtable h)
  {
    Vector v = new Vector();
    for(Enumeration e = h.keys() ; e.hasMoreElements() ;)
    {
      String s = (String)e.nextElement();
      v.add(s);
      v.add(h.get(s));
    }

    return v;
  }


  /**
  *
  * Automatically updating status manager with
  * BatchUpdateTimer thread
  *  @return 	true if automatically updating 
  *
  */
  public boolean isAutoUpdate()
  {
    return autoUpdates;
  }

  /**
  *
  * Automatically updating status manager with
  * BatchUpdateTimer thread
  * @param 	true if automatically updating 
  *
  */
  public void setAutoUpdate(boolean b)
  {
    autoUpdates = b;
  }

  /**
  *
  * Updates the mode on the combo box to reflect the current state
  *
  */
  public void updateMode() 
  {
    if (jobComboBox != null) 
      jobComboBox.setSelectedItem(mysettings.getCurrentMode());
  }

  /**
  *
  * Updates the mode on the combo box to reflect the
  * requested value
  * @param s	mode (batch /interactive) to set to
  *
  */
  public void updateMode(String s) 
  {
    mysettings.setCurrentMode(s);
    if (jobComboBox != null) 
      jobComboBox.setSelectedItem(mysettings.getCurrentMode());
  }


  /**
  *
  * A panel to show the status of any jobs and to view them,
  * and to set the mode.
  * @param f 	parent frame, to which dialogs will be attached.
  *
  */
  public JPanel statusPanel(final JFrame f) 
  {
    final JPanel jobPanel = new JPanel(new BorderLayout());

    ClassLoader cl = this.getClass().getClassLoader();
    ImageIcon jobIcon = new ImageIcon(cl.getResource("images/Job_manager_button.gif"));
    JLabel jobLabel = new JLabel(jobIcon);
    jobLabel.setToolTipText("Batch Job Manager");

    jobPanel.add(jobLabel,BorderLayout.WEST);
    jobButton = new JButton("(No Current Jobs)");
    jobButton.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        jobPanel.setCursor(new Cursor(Cursor.WAIT_CURSOR));
        showPendingResults(f);
        jobPanel.setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
      }
    });
    jobPanel.add(jobButton,BorderLayout.CENTER);
    jobComboBox = new JComboBox(mysettings.modeVector());

    updateMode();
    jobComboBox.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e) 
      {
        JComboBox cb = (JComboBox)e.getSource();
        String modeName = (String)cb.getSelectedItem();
        mysettings.setCurrentMode(modeName);
      }
    });
    jobPanel.add(jobComboBox,BorderLayout.EAST);

    Dimension d = jobPanel.getPreferredSize();
    d = new Dimension((int)d.getWidth(),
                      jobIcon.getIconHeight()-2);

    jobPanel.setPreferredSize(d);
    return jobPanel;
  }

  /**
  *
  * Display the list of pending and completed batch results
  * window
  *
  */
  public void showPendingResults(JFrame f) 
  {
    if ((completed_jobs == 0) && (running_jobs == 0)) 
    {
      JOptionPane.showMessageDialog(f,"You can only view pending results\n"
				    + "if batch/background jobs have been\n"
				    + "submitted in the current session.");
    } 
    else 
    {
      try
      {
        new ShowSavedResults(mysettings, this);
      } 
      catch (JembossSoapException eae) 
      {
        new AuthPopup(mysettings,f);
      }
    }
  } 

  /**
  *
  * @return job status text
  *
  */
  public String getStatus()
  {
    return jobButton.getText();
  }

}

