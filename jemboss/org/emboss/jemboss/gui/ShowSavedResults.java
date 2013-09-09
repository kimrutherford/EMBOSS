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


package org.emboss.jemboss.gui;

import java.awt.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;

import org.emboss.jemboss.programs.*;
import org.emboss.jemboss.soap.*;
import org.emboss.jemboss.Jemboss;
import org.emboss.jemboss.JembossParams;

/**
*
* Shows a list of results from the SOAP server
* and displays individual result sets
*
*/
public class ShowSavedResults extends JFrame
{

  /** busy cursor */
  private Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR); 
  /** done cursor */
  private Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);
  /** data list model */
  private DefaultListModel datasets = new DefaultListModel();
  /** result data list panel */
  private ScrollPanel sp = new ScrollPanel();
  /** text area for initial description of the saved results
      window and the run information of the results */
  private JTextArea aboutRes; 
  /** scrollpane for description of the results */
  private JScrollPane aboutScroll;
  /** scroll panel for result data */
  private JScrollPane ss;
  /** result status panel */
  private JPanel resButtonStatus;
  /** status field */
  private JTextField statusField;
  /** manu bar for results panel */
  private JMenuBar resMenu = new JMenuBar();
  /** refresh image */
  private ImageIcon rfii;


  /**
  *
  * @param frameName	title name for frame
  *
  */
  public ShowSavedResults(String frameName)
  {
    super(frameName);
    aboutRes = new JTextArea("Select a result set from"
                              +"\nthose listed and details"
                              +"\nof that analysis will be"
                              +"\nshown here. Then you can"
                              +"\neither delete or view those"
                              +"\nresults using the buttons below.");
    aboutScroll = new JScrollPane(aboutRes);  
    ss = new JScrollPane(sp);
    ss.getViewport().setBackground(Color.white);

//  resMenu.setLayout(new FlowLayout(FlowLayout.LEFT,10,1));
    ClassLoader cl = getClass().getClassLoader();
    rfii = new ImageIcon(cl.getResource("images/Refresh_button.gif"));

//results status
    resButtonStatus = new JPanel(new BorderLayout());
    Border loweredbevel = BorderFactory.createLoweredBevelBorder();
    Border raisedbevel = BorderFactory.createRaisedBevelBorder();
    Border compound = BorderFactory.createCompoundBorder(raisedbevel,loweredbevel);
    statusField = new JTextField();
    statusField.setBorder(compound);
    statusField.setEditable(false);
  }


  /**
  *
  * Show the saved results on the server.
  * @param mysettings	jemboss settings
  * @param frameName    title name for frame
  *
  */
  public ShowSavedResults(final JembossParams mysettings, final JFrame f)
  {

    this("Saved results list"+(Jemboss.withSoap ? " on server":""));
     
    try
    {
      final ResultList reslist = new ResultList(mysettings);
      JMenu resFileMenu = new JMenu("File");
      resMenu.add(resFileMenu);

      final JCheckBoxMenuItem listByProgram = 
                 new JCheckBoxMenuItem("List by program");
      listByProgram.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          listByProgramName();
        }
      });
      resFileMenu.add(listByProgram);

      JCheckBoxMenuItem listByDate = 
                 new JCheckBoxMenuItem("List by date",true);
      listByDate.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          listByDateRun(reslist,false);
        }
      });
      resFileMenu.add(listByDate);

      ButtonGroup group = new ButtonGroup();
      group.add(listByProgram);
      group.add(listByDate);

      JButton refresh = new JButton(rfii);
      refresh.setMargin(new Insets(0,1,0,1));
      refresh.setToolTipText("Refresh");
      resMenu.add(refresh);

      refresh.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e) 
        {
          try
          {
            setCursor(cbusy);
            ResultList newlist = new ResultList(mysettings);
            setCursor(cdone);
            if (newlist.getStatus().equals("0")) 
            {
              reslist.updateRes(newlist.hash());
              datasets.removeAllElements();

              StringTokenizer tok = new StringTokenizer(
                         (String)reslist.get("list"), "\n");
              while (tok.hasMoreTokens())
                datasets.addElement(convertToPretty(tok.nextToken()));

              if(listByProgram.isSelected())
                listByProgramName();
              else
                listByDateRun(reslist,false);
            } 
            else 
            {
              JOptionPane.showMessageDialog(null,
                     newlist.getStatusMsg(), "Soap Error",
                              JOptionPane.ERROR_MESSAGE);
            }
          } 
          catch (JembossSoapException eae) 
          {
            AuthPopup ap = new AuthPopup(mysettings,f);
            ap.setBottomPanel();
            ap.setSize(380,170);
            ap.pack();
            ap.setVisible(true);
          }
        }
      });


      resFileMenu.addSeparator();
      JMenuItem resFileMenuExit = new JMenuItem("Close");
      resFileMenuExit.setAccelerator(KeyStroke.getKeyStroke(
                    KeyEvent.VK_E, ActionEvent.CTRL_MASK));

      resFileMenuExit.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e) 
        {
          dispose();
        }
      });
      resFileMenu.add(resFileMenuExit);
      setJMenuBar(resMenu);
        
      // this is the list of saved results
      listByDateRun(reslist,true);
  
      final JList st = new JList(datasets);
      st.setCellRenderer(new TabListCellRenderer());

      st.addListSelectionListener(new ListSelectionListener()
      {
        public void valueChanged(ListSelectionEvent e) 
        {
          if (e.getValueIsAdjusting())
            return;

          JList theList = (JList)e.getSource();
          if (!theList.isSelectionEmpty()) 
          {
            int index = theList.getSelectedIndex();
            String thisdata = convertToOriginal(datasets.elementAt(index));
            aboutRes.setText((String)reslist.get(thisdata));
            aboutRes.setCaretPosition(0);
          }
        }
      });

      st.addMouseListener(new MouseAdapter() 
      {
        public void mouseClicked(MouseEvent e) 
        {
          if (e.getClickCount() == 2) 
          {
            try
            {
              setCursor(cbusy);
              String project = convertToOriginal(st.getSelectedValue());
              ResultList thisres = new ResultList(mysettings,project,
                                              "show_saved_results");
              new ShowResultSet(thisres.hash(),project,mysettings);
              setCursor(cdone);
            } 
            catch (JembossSoapException eae) 
            {  
              AuthPopup ap = new AuthPopup(mysettings,f);
              ap.setBottomPanel();
              ap.setSize(380,170);
              ap.pack();
              ap.setVisible(true);
            }
          }
        }
      });
      sp.add(st);
        
      // display retrieves all files and shows them in a window
      JPanel resButtonPanel = new JPanel();
      JButton showResButton = new JButton("Display");
      showResButton.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e) 
	{
          String sel = convertToOriginal(st.getSelectedValue());
  	  if(sel != null)
          {
	    try 
	    {
	      setCursor(cbusy);
	      ResultList thisres = new ResultList(mysettings,
                                   sel,"show_saved_results");
	      if(thisres.hash().size()==0)
	          JOptionPane.showMessageDialog(sp,
              "This application launch '"+sel+
              "' didn't produce any result files.");
	      else
	      new ShowResultSet(thisres.hash(),sel,mysettings);
	    } 
	    catch (JembossSoapException eae)
	    {
	    	if(mysettings.getUseAuth())
	    	{         	   
	    		AuthPopup ap = new AuthPopup(mysettings,f);
	    		ap.setBottomPanel();
	    		ap.setSize(380,170);
	    		ap.pack();
	    		ap.setVisible(true);
	    	}
	    	else
	    	{
	    		JOptionPane.showMessageDialog(null,
	    				"Problem while getting result list for '"+
	    						sel+"' :"+eae.getMessage(),"Warning",
	    						JOptionPane.ERROR_MESSAGE);
	    	}
	    }
	    finally
	    {
	    	setCursor(cdone);	    	
	    }
  	  } 
  	  else 
  	  {
            statusField.setText("Nothing selected to be displayed.");
  	  }
        }
      });

      // add a users note for that project
      JButton addNoteButton = new JButton("Edit Notes");
      addNoteButton.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          String sel = convertToOriginal(st.getSelectedValue());
          if(sel != null)
          {
            try
            {
              setCursor(cbusy);
              ResultList thisres = new ResultList(mysettings,
                           sel,"Notes","show_saved_results");
              new ShowResultSet(thisres.hash(),sel,mysettings);
              setCursor(cdone);
            }
            catch (JembossSoapException eae)
            {
              AuthPopup ap = new AuthPopup(mysettings,f);
              ap.setBottomPanel();
              ap.setSize(380,170);
              ap.pack();
              ap.setVisible(true);
            }
          }
          else
          {
            statusField.setText("Select a project!");
          }
        }
      });
        
      // delete removes the file on the server & edits the list
      JButton delResButton = new JButton("Delete");
      delResButton.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e) 
        {
          Object sel[] = st.getSelectedValues();
          if(sel != null) 
          {
            String selList= new String("");
            JTextPane delList = new JTextPane();
            FontMetrics fm = delList.getFontMetrics(delList.getFont());
            int maxWidth = 0;
            for(int i=0;i<sel.length;i++)
            {
              if(i==sel.length-1)
                selList=selList.concat((String)sel[i]);
              else
                selList=selList.concat(sel[i]+"\n");

              int width = fm.stringWidth((String)sel[i]);
              if(width>maxWidth)
                maxWidth=width;
            }
            int ok = JOptionPane.OK_OPTION;
            if(sel.length>1)
            {
              JScrollPane scrollDel = new JScrollPane(delList);
              delList.setText(selList);
              delList.setEditable(false);
              delList.setCaretPosition(0);

              Dimension d1 = delList.getPreferredSize();
              int maxHeight = (int)d1.getHeight()+5;
              if(maxHeight > 350)
                maxHeight = 350;
              else if(maxHeight < 50)
                maxHeight = 50;
              
              scrollDel.setPreferredSize(new Dimension(maxWidth+30,maxHeight));

              ok = JOptionPane.showConfirmDialog(null,
                         scrollDel,"Confirm Deletion",
                  JOptionPane.YES_NO_OPTION);
            }
            if(ok == JOptionPane.OK_OPTION)
            {
              try        // ask the server to delete these results
	      {
	        setCursor(cbusy); 
                selList = convertToOriginal(selList);
	        new ResultList(mysettings,selList,
                                         "delete_saved_results"); 
	        setCursor(cdone);
	       
                // amend the list
                for(int i=0;i<sel.length;i++)
                  datasets.removeElement(sel[i]);
                
                statusField.setText("Deleted " + sel.length + "  result(s)");
     
	        aboutRes.setText("");
	        st.setSelectedIndex(-1);
	      } 
	      catch (JembossSoapException eae) 
	      {
                AuthPopup ap = new AuthPopup(mysettings,f);
                ap.setBottomPanel();
                ap.setSize(380,170);
                ap.pack();
                ap.setVisible(true);
              }
            }
	  } 
          else 
          {
            statusField.setText("Nothing selected for deletion.");
	  }
	}
      });
      resButtonPanel.add(delResButton);
      resButtonPanel.add(addNoteButton);
      resButtonPanel.add(showResButton);
      resButtonStatus.add(resButtonPanel, BorderLayout.CENTER);
      resButtonStatus.add(statusField, BorderLayout.SOUTH);

      Container c = getContentPane();
      c.add(ss,BorderLayout.WEST);
      c.add(aboutScroll,BorderLayout.CENTER);
      c.add(resButtonStatus,BorderLayout.SOUTH);
      pack();
      
      setVisible(true);
    } 
    catch (JembossSoapException eae) 
    {
      AuthPopup ap = new AuthPopup(mysettings,f);
      ap.setBottomPanel();
      ap.setSize(380,170);
      ap.pack();
      ap.setVisible(true);
    }

  }


  /**
  *
  * Show the results sent to a batch queue.
  * @param mysettings   jemboss settings
  * @param epr		pending results
  * @throws JembossSoapException when server connection fails
  *
  */
  public ShowSavedResults(final JembossParams mysettings, final PendingResults epr)
                                           throws JembossSoapException
  {
    this("Current Sessions Results");

    Dimension d = new Dimension(270,100);
    ss.setPreferredSize(d);
//  ss.setMaximumSize(d);

    JMenu resFileMenu = new JMenu("File");
    resMenu.add(resFileMenu);

    JButton refresh = new JButton(rfii);
    refresh.setMargin(new Insets(0,1,0,1));
    refresh.setToolTipText("Refresh");
    resMenu.add(refresh);
    refresh.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e) 
      {
	setCursor(cbusy);
	epr.updateStatus();
	setCursor(cdone);
	datasets.removeAllElements();
	Enumeration enumer = epr.descriptionHash().keys();
	while (enumer.hasMoreElements()) 
        {
	  String image = convertToPretty((String)enumer.nextElement());
	  datasets.addElement(image);
	}
      }
    });

    JMenuItem resFileMenuExit = new JMenuItem("Close");
    resFileMenuExit.setAccelerator(KeyStroke.getKeyStroke(
                    KeyEvent.VK_E, ActionEvent.CTRL_MASK));

    resFileMenuExit.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e) 
      {
	dispose();
      }
    });
    resFileMenu.add(resFileMenuExit);
    setJMenuBar(resMenu);
    
    // set up the results list in the gui
    Enumeration enumer = epr.descriptionHash().keys();
    while (enumer.hasMoreElements()) 
      datasets.addElement(convertToPretty((String)enumer.nextElement()));

    final JList st = new JList(datasets);
    st.setCellRenderer(new TabListCellRenderer());
    st.addListSelectionListener(new ListSelectionListener()
    {
      public void valueChanged(ListSelectionEvent e) 
      {
	if (e.getValueIsAdjusting())
	  return;
	
	JList theList = (JList)e.getSource();
	if (!theList.isSelectionEmpty()) 
        {
	  int index = theList.getSelectedIndex();
	  String thisdata = convertToOriginal(datasets.elementAt(index));
	  aboutRes.setText((String)epr.descriptionHash().get(thisdata));
      	  aboutRes.setCaretPosition(0);
	  aboutRes.setEditable(false);
	}
      }
    });


    st.addMouseListener(new MouseAdapter() 
    {
      public void mouseClicked(MouseEvent e) 
      {
	if (e.getClickCount() == 2) 
        {
	  try
          {
	    setCursor(cbusy);
            String project = convertToOriginal(st.getSelectedValue());
	    ResultList thisres = new ResultList(mysettings,project, 
                                             "show_saved_results");
	    setCursor(cdone);
	    if(thisres.getStatus().equals("0")) 
              new ShowResultSet(thisres.hash(),project,mysettings);
            else 
              JOptionPane.showMessageDialog(null,
                     thisres.getStatusMsg(), "Soap Error",
                              JOptionPane.ERROR_MESSAGE);
 
	  } 
          catch (JembossSoapException eae) 
          {
            AuthPopup ap = new AuthPopup(mysettings,null);
            ap.setBottomPanel();
            ap.setSize(380,170);
            ap.pack();
            ap.setVisible(true);
	  }
	}
      }
    });
    sp.add(st);
    
    // display retrieves all the files and shows them in a window
   
    JPanel resButtonPanel = new JPanel();
    JButton showResButton = new JButton("Display");
    showResButton.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent e) 
      {
	if(st.getSelectedValue() != null) 
        {
	  try
          {
	    setCursor(cbusy);
            String project = convertToOriginal(st.getSelectedValue());
	    ResultList thisres = new ResultList(mysettings,project, 
                                                "show_saved_results");
	    setCursor(cdone);
	    if(thisres.getStatus().equals("0")) 
              new ShowResultSet(thisres.hash(),project,mysettings);
            else 
              JOptionPane.showMessageDialog(null,
                     thisres.getStatusMsg(), "Soap Error",
                              JOptionPane.ERROR_MESSAGE);
	  } 
          catch (JembossSoapException eae) 
          {
            setCursor(cdone);
            AuthPopup ap = new AuthPopup(mysettings,null);
            ap.setBottomPanel();
            ap.setSize(380,170);
            ap.pack();
            ap.setVisible(true);
	  }
	}
      }
    });
    
    // delete removes the file on the server and edits the list
    JButton delResButton = new JButton("Delete");
    delResButton.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e) 
      {
        Object sel[] = st.getSelectedValues();
	if(sel != null) 
        {
          String selList= new String("");
          for(int i=0;i<sel.length;i++)
            selList=selList.concat(sel[i]+"\n");

          int ok = JOptionPane.OK_OPTION;
          if(sel.length>1)
            ok = JOptionPane.showConfirmDialog(null,
                "Delete the following results:\n"+selList,
                "Confirm Deletion",
                JOptionPane.YES_NO_OPTION);
           
          if(ok == JOptionPane.OK_OPTION)
          {

	    try 
            {
	      setCursor(cbusy);
              selList = convertToOriginal(selList);
	      new ResultList(mysettings,selList,
                                             "delete_saved_results");
	      setCursor(cdone);

              for(int i=0;i<sel.length;i++)
              {
                JembossProcess jp = epr.getResult(convertToOriginal(sel[i]));
                epr.removeResult(jp);
                datasets.removeElement(sel[i]);  // amend the list
              }
              statusField.setText("Deleted " + sel.length+ "  result(s)");
	      aboutRes.setText("");
	      st.setSelectedIndex(-1);
	    }
            catch (JembossSoapException eae)
            {
	      // shouldn't happen
              AuthPopup ap = new AuthPopup(mysettings,null);
              ap.setBottomPanel();
              ap.setSize(380,170);
              ap.pack();
              ap.setVisible(true);
	    }
          }
	}
      }
    });
    resButtonPanel.add(delResButton);
    resButtonPanel.add(showResButton);
    resButtonStatus.add(resButtonPanel, BorderLayout.CENTER);
    resButtonStatus.add(statusField, BorderLayout.SOUTH);

    Container c = getContentPane();
    c.add(ss,BorderLayout.WEST);
    c.add(aboutScroll,BorderLayout.CENTER);
    c.add(resButtonStatus,BorderLayout.SOUTH);
    setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
    
    pack();
    setVisible(true);

//add in automatic updates
    String freq = (String)AdvancedOptions.jobMgr.getSelectedItem();
    int ind = freq.indexOf(" ");
    new ResultsUpdateTimer(Integer.parseInt(freq.substring(0,ind)),
                           datasets, this);
    statusField.setText("Window refresh rate " + freq);

  }

  /**
  *
  * List results by date
  * @param reslist	result list
  * @param ldisp	
  *
  */
  private void listByDateRun(ResultList reslist,boolean ldisp)
  {
    StringTokenizer tokenizer =
         new StringTokenizer((String)reslist.get("list"), "\n");

    Vector vdata = new Vector();
    while (tokenizer.hasMoreTokens())
    {
      String data = convertToPretty(tokenizer.nextToken());
      if(datasets.contains(data) || ldisp)
        vdata.add(data);
    }
    datasets.removeAllElements();

    Enumeration en = vdata.elements();
    while(en.hasMoreElements())
      datasets.addElement(en.nextElement());

  }

  /**
  *
  * List results by alphabetical name
  *
  */
  private void listByProgramName()
  { 
    int nresult = datasets.size();
    String res[] = new String[nresult];
    for(int i=0;i<nresult;i++)
      res[i] = (String)datasets.getElementAt(i);
    Arrays.sort(res);
    datasets.removeAllElements();
    for(int i=0;i<nresult;i++)
      datasets.addElement(res[i]);
  }

  /**
  *
  * Convert the project names to remove the underscores
  * ('_') with blank spaces 
  * @param sorig	original string to convert 
  *
  */
  public static String convertToPretty(String sorig)
  {
//    int index = sorig.indexOf('_');
//  if(index > -1)
//    sorig = sorig.substring(0,index) + "\t" +
//            sorig.substring(index+1);
    return sorig.replace('_',' ');
  }

  /**
  *
  * Convert back to the original project names replace
  * blank spaces with underscores ('_') 
  * @param sorig        string to convert 
  *
  */
  private String convertToOriginal(Object sorig)
  {
    if(sorig==null)
        return null;
      
    String s = ((String)sorig).replace('\t','_');
    return s.replace(' ','_');
  }

}


/**
*
* Adapted from an example at
* http://www.spindoczine.com/sbe/
*
*/
class TabListCellRenderer extends JLabel implements ListCellRenderer
{
  protected static Border m_noFocusBorder;
  protected FontMetrics m_fm = null;
  protected Insets m_insets = new Insets(0, 0, 0, 0);

  protected int m_defaultTab = 50;
  protected int[] m_tabs = null;

  public TabListCellRenderer()
  {
    super();
    m_noFocusBorder = new EmptyBorder(1, 1, 1, 1);
    setOpaque(true);
    setBorder(m_noFocusBorder);
  }

  public Component getListCellRendererComponent(JList list,
              Object value, int index, boolean isSelected, boolean cellHasFocus)     
  {         
    setText(value.toString());

    setBackground(isSelected ? list.getSelectionBackground() : list.getBackground());
    setForeground(isSelected ? list.getSelectionForeground() : list.getForeground());
                
    setFont(list.getFont());
    setBorder((cellHasFocus) ? UIManager.getBorder("List.focusCellHighlightBorder") : m_noFocusBorder);

    return this;
  }

  public void setDefaultTab(int defaultTab) { m_defaultTab = defaultTab; }
  public int getDefaultTab() { return m_defaultTab; }
  public void setTabs(int[] tabs) { m_tabs = tabs; }
  public int[] getTabs() { return m_tabs; }
  public int getTab(int index)
  {
    if(m_tabs == null)
      return m_defaultTab*index;
            
    int len = m_tabs.length;
    if(index >= 0 && index < len)
      return m_tabs[index];

    return m_tabs[len-1] + m_defaultTab*(index-len+1);
  }


  public void paint(Graphics g)
  {
    m_fm = g.getFontMetrics();
      
    g.setColor(getBackground());
    g.fillRect(0, 0, getWidth(), getHeight());
      getBorder().paintBorder(this, g, 0, 0, getWidth(), getHeight());

    g.setColor(getForeground());
    g.setFont(getFont());
    m_insets = getInsets();
    int x = m_insets.left;
    int y = m_insets.top + m_fm.getAscent();

    StringTokenizer st = new StringTokenizer(getText(), "\t");
    while (st.hasMoreTokens()) 
    {
      String sNext = st.nextToken();
      g.drawString(sNext, x, y);
      x += m_fm.stringWidth(sNext);

      if (!st.hasMoreTokens())
        break;
      int index = 0;
      while (x >= getTab(index))
      index++;
      x = getTab(index);
    }
  }

}


