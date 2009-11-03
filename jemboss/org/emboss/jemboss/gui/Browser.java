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


import javax.swing.*;
import javax.swing.event.*;
import javax.swing.tree.*;
import java.awt.*;
import java.awt.geom.*;
import java.awt.event.*;
import java.net.*;
import java.io.*;
import java.util.Vector;
import javax.swing.border.*;

import org.emboss.jemboss.JembossParams;


/**
*
* Jemboss web browser
*
*/
public class Browser extends JFrame
                     implements HyperlinkListener, ActionListener
{

  /** status field */
  private JTextField statusField;
  /** URL cache combo field */
  private MemoryComboBox urlField;
  /** HTML pane   */
  private JEditorPane htmlPane;
  /** initial URL */
  private String initialURL;
  /** busy cursor */
  private Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
  /** done cursor */
  private Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);
  /** Help topics */
  private String topics[] = { "Home", "About",
                              "User Guide",
                              "File Manager", 
                              "Results Manager", 
                              "Sequence List", 
                              "Alignment Editor"};
  private String embossTopics[] =
                            { "Home", "Apps"};

  /** JSplitPane sp */
  private JSplitPane sp;
  /** Back menu option */
  private JMenuItem backMenu;
  /** Back button option */
  private JButton backBt;
  /** Forward menu option */
  private JMenuItem fwdMenu;
  /** Forward button option */
  private JButton fwdBt;

  /**
  *
  * @param initialURL	initial URL
  * @param name		browser frame title
  * @param mysettings	jemboss settings
  *
  */
  public Browser(String initialURL, String name, 
                 JembossParams mysettings) throws IOException
  {
    this(initialURL,name,false,"",mysettings);
  }


  /**
  *
  * @param initialURL   initial URL
  * @param name         browser frame title
  * @param ltext	true if html as string past to web browser 
  * @param text		html as string
  * @param mysettings   jemboss settings
  *
  */
  public Browser(String initialURL, String name,  boolean ltext, 
                 String text, JembossParams mysettings) throws IOException
  {
    super(name);
    this.initialURL = initialURL;

    if(mysettings.isBrowserProxy())
    {
      System.setProperty("http.proxyHost",mysettings.getBrowserProxyHost());
      System.setProperty("http.proxyPort", Integer.toString(
                          mysettings.getBrowserProxyPort()));
   
      System.setProperty("proxyHost",mysettings.getBrowserProxyHost());
      System.setProperty("proxyPort", Integer.toString(
                          mysettings.getBrowserProxyPort()));
    }

    if(ltext)        
    {
      htmlPane = new JEditorPane();
      if( (text.indexOf("<html>") > -1) ||
          (text.indexOf("<HTML>") > -1) )
        htmlPane.setContentType("text/html");
      htmlPane.setText(text);
      htmlPane.addHyperlinkListener(this);
      setBrowserSize();
      Vector urlCache = new Vector();
      urlCache.add(name+".html");
      setUpJMenuBar(urlCache);
      addToScrollPane();
      setVisible(true);
    }
    else
    {
      URL pageURL = new URL(initialURL);
      setURL(pageURL,initialURL);
    }
  }


  /**
  *
  * @param urlName	URL to display
  * @param initialURL   initial URL
  *
  */
  public Browser(URL urlName, String initialURL) throws IOException
  {
    super(initialURL);
    this.initialURL = initialURL;
    setURL(urlName,initialURL);
  }


  /**
  *
  * Set the URL in the browser
  * @param url		URL to display
  * @param name 	URL name
  *
  */
  public void setURL(URL url, String name)
  {
    try
    {
      htmlPane = new JEditorPane(url);
      htmlPane.addHyperlinkListener(this);

      Vector urlCache = new Vector();
      urlCache.add(url);
      setBrowserSize();
      setUpJMenuBar(urlCache);
      setTitle(name);
      addToScrollPane();
      setVisible(true);
    }
    catch(IOException ioe)
    {
      JOptionPane.showMessageDialog(null,
                              "Cannot Load URL\n"+name,
                              "Error", JOptionPane.ERROR_MESSAGE);
    }
  }


  /**
  *
  * Method to create the frames menu and tool bar.
  * @param urlCache	URL cache
  *
  */
  private void setUpJMenuBar(Vector urlCache)
  {
    JMenuBar menuBar = new JMenuBar();
    JToolBar toolBarURL  = new JToolBar();
    JToolBar toolBarIcon = new JToolBar();

    JMenu fileMenu = new JMenu("File");
    fileMenu.setMnemonic(KeyEvent.VK_F);
    menuBar.add(fileMenu);

    // back
    backMenu = new JMenuItem("Back");
    backMenu.setAccelerator(KeyStroke.getKeyStroke(
              KeyEvent.VK_B, ActionEvent.CTRL_MASK));
    backMenu.setActionCommand("BACK");
    backMenu.addActionListener(this);
    fileMenu.add(backMenu);
    backMenu.setEnabled(false);

    fwdMenu = new JMenuItem("Forward");
    fwdMenu.setAccelerator(KeyStroke.getKeyStroke(
              KeyEvent.VK_F, ActionEvent.CTRL_MASK));
    fwdMenu.setActionCommand("FWD");
    fwdMenu.addActionListener(this);
    fileMenu.add(fwdMenu);

    // close
    fileMenu.addSeparator();
    JMenuItem closeMenu = new JMenuItem("Close");
    closeMenu.setAccelerator(KeyStroke.getKeyStroke(
              KeyEvent.VK_E, ActionEvent.CTRL_MASK));

    closeMenu.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        setVisible(false);
      }
    });
    fileMenu.add(closeMenu);

    // view
    JMenu viewMenu = new JMenu("View");
    viewMenu.setMnemonic(KeyEvent.VK_V);
    menuBar.add(viewMenu);
    JCheckBoxMenuItem sideTopics = new JCheckBoxMenuItem(
                                       "Display help topics");
    sideTopics.setSelected(true);
    sideTopics.setAccelerator(KeyStroke.getKeyStroke(
              KeyEvent.VK_L, ActionEvent.CTRL_MASK));
    sideTopics.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        if(sp.getDividerLocation() > 5)
          sp.setDividerLocation(0);
        else
          sp.setDividerLocation(100);
      }
    });
    viewMenu.add(sideTopics);

    // jemboss logo button
    ClassLoader cl = this.getClass().getClassLoader();
    ImageIcon jem = new ImageIcon(cl.getResource(
                               "images/Jemboss_logo_small.gif"));
    JIconButton jembossButton = new JIconButton(jem);
    jembossButton.addActionListener(this);
    jembossButton.setActionCommand("JEMBOSS");

    // url field
    JLabel urlLabel = new JLabel("URL:");
    urlField = new MemoryComboBox(urlCache);
    urlField.addActionListener(this);
    int urlFieldHeight = (int)urlField.getPreferredSize().getHeight();

// Icon tool bar
    // Back JButton
    backBt = new JButton()
    {
      public void paintComponent(Graphics g)
      {
        super.paintComponent(g);
        Graphics2D g2 = (Graphics2D)g;

        g2.setColor(new Color(0,128,0));

        float loc1[][] = { {4,12}, {14,22}, {14,16}, 
                                   {18,16}, {18,12} };
        g2.fill(Browser.makeShape(loc1));
        g2.setColor(Color.green);

        float loc2[][] = { {4,12}, {14,2}, {14,8}, 
                                   {18,8}, {18,12} };
        g2.fill(Browser.makeShape(loc2));

        if(!isEnabled())
        {
          g2.setColor(Color.gray);
          float loc3[][] = { {5,12}, {14,21}, {14,15}, 
                                     {18,15}, {18,12} };
          g2.fill(Browser.makeShape(loc3));
          g2.setColor(Color.lightGray);
          float loc4[][] = { {5,12}, {14,3}, {14,9}, 
                                     {18,9}, {18,12} };
          g2.fill(Browser.makeShape(loc4));
        }
        setSize(22,24);
      }
    };
    Dimension dBut = new Dimension(22,24);
    backBt.setPreferredSize(dBut);
    backBt.setMaximumSize(dBut);
    backBt.setPreferredSize(new Dimension(15,15));
    backBt.setActionCommand("BACK");
    backBt.addActionListener(this);
    backBt.setEnabled(false);

    // Forward JButton
    fwdBt = new JButton()
    {
      public void paintComponent(Graphics g)
      {
        super.paintComponent(g);
        Graphics2D g2 = (Graphics2D)g;

        g2.setColor(new Color(0,128,0));

        float loc1[][] = { {4,12}, {4,16}, {8,16}, 
                                   {8,22}, {18,12} };

        g2.fill(Browser.makeShape(loc1));
        g2.setColor(Color.green);

        float loc2[][] = { {4,12}, {4,8}, {8,8},
                                   {8,2}, {18,12} };
        g2.fill(Browser.makeShape(loc2));

        if(!isEnabled())
        {
          g2.setColor(Color.gray);
          float loc3[][] = { {4,12}, {4,15}, {8,15},
                                     {8,21}, {17,12} };
          g2.fill(Browser.makeShape(loc3));
          g2.setColor(Color.lightGray);
          float loc4[][] = { {4,12}, {4,7}, {8,7},
                                     {8,3}, {17,12} };
          g2.fill(Browser.makeShape(loc4));
        }

        setSize(22,24);
      }
    };
    fwdBt.setPreferredSize(dBut);
    fwdBt.setMaximumSize(dBut);
    fwdBt.setActionCommand("FWD");
    fwdBt.addActionListener(this);
    fwdBt.setEnabled(false);

    toolBarIcon.add(backBt);
    toolBarIcon.add(fwdBt);
    toolBarIcon.add(jembossButton);

    toolBarURL.add(urlLabel);
    toolBarURL.add(urlField);

    setJMenuBar(menuBar);
   
    JPanel toolBars = new JPanel(new BorderLayout());
    toolBars.add(toolBarIcon, BorderLayout.NORTH);
    toolBars.add(toolBarURL, BorderLayout.SOUTH);
    getContentPane().add(toolBars, BorderLayout.NORTH);

    int urlFieldWidth  = (int)toolBarURL.getPreferredSize().getWidth();
    Dimension d = new Dimension(urlFieldWidth,urlFieldHeight);
    urlField.setMaximumSize(d);

    int iconBarWidth  = (int)toolBarIcon.getPreferredSize().getWidth();
    int iconBarHeight = jem.getIconHeight();
    d = new Dimension(iconBarWidth,iconBarHeight);
    toolBarIcon.setPreferredSize(d);
  }


  /**
  *
  * Set the Jemboss web browser size
  *
  */
  private void setBrowserSize()
  {
    Dimension screenSize = getToolkit().getScreenSize();
    int width  = (int)(screenSize.width * 0.6);
    int height = (int)(screenSize.height * 0.85);
    setBounds((int)(width*.59), (int)(height*.02), width, height);
  }


  /**
  *
  * Add the html pane to a scrollpane, list and splitpane and set the
  * size of the html pane
  *
  */
  private void addToScrollPane()
  {
    htmlPane.setEditable(false);
    htmlPane.setCaretPosition(0);
    
    Box bacross = Box.createHorizontalBox();
    bacross.add(Box.createHorizontalGlue());
    JButton listClose = new JButton()
    {
      public void paintComponent(Graphics g)
      {
        super.paintComponent(g);
        BasicStroke stroke = new BasicStroke(2.0f);
        Graphics2D g2 = (Graphics2D) g;

        g2.setStroke(stroke);
        g2.setColor(Color.gray);
        g2.drawLine(4,5,9,10);
        g2.drawLine(9,5,4,10);

        g2.setColor(Color.black);
        g2.drawLine(4,4,9,9);
        g2.drawLine(9,3,3,9);
        setSize(15,15);
      }
    };
    listClose.addActionListener(this);
    listClose.setActionCommand("CLOSE");
    listClose.setPreferredSize(new Dimension(15,15));
    bacross.add(listClose);

    JPanel leftPane = new JPanel(new BorderLayout());
    JScrollPane leftScroll = new JScrollPane(getWebScape());
    leftPane.add(bacross, BorderLayout.NORTH);
    leftPane.add(leftScroll, BorderLayout.CENTER);
    leftScroll.getViewport().setBackground(Color.white);
 

    sp = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT,
		        leftPane, htmlPane);
    sp.setDividerSize(8);
    sp.setDividerLocation(100);
    sp.setOneTouchExpandable(true);
    JScrollPane scrollPane = new JScrollPane(htmlPane);
    sp.add(scrollPane);

    // ensures html wraps properly
    htmlPane.setPreferredSize(getPreferredSize());
    getContentPane().add(sp, BorderLayout.CENTER);  

    // status field
    Border loweredbevel = BorderFactory.createLoweredBevelBorder();
    Border raisedbevel = BorderFactory.createRaisedBevelBorder();
    Border compound = BorderFactory.createCompoundBorder(raisedbevel,loweredbevel);
    statusField = new JTextField();
    statusField.setBorder(compound);
    statusField.setEditable(false);
    getContentPane().add(statusField, BorderLayout.SOUTH);
  }

  /**
  *
  * Override actionPerformed
  * @param event 	action event
  *
  */
  public void actionPerformed(ActionEvent event) 
  {
    URL url = null;
    setCursor(cbusy);
    if (event.getSource() == urlField) 
    {
      Object select = urlField.getSelectedItem();
      if(select instanceof String)
      {
        try
        {
          url = new URL((String)select);
        }
        catch(MalformedURLException me){}
      }
      else
        url = (URL)select;
    }
    else if (event.getActionCommand().equals("JEMBOSS"))
    {
      try
      {
        url = new URL("http://emboss.sourceforge.net/Jemboss/");
      }
      catch(MalformedURLException me){}
    }
    else if (event.getActionCommand().equals("BACK"))
    {
      int index = urlField.getIndexOf(urlField.getSelectedItem())-1;
      if(index > -1 && index < urlField.getItemCount())
        url = urlField.getURLAt(index);
    }
    else if (event.getActionCommand().equals("FWD"))
    {
      int index = urlField.getIndexOf(urlField.getSelectedItem())+1;
      if(index > -1 && index < urlField.getItemCount())
        url = urlField.getURLAt(index);
    }
    else if (event.getActionCommand().equals("CLOSE"))
    {
      setCursor(cdone);
      sp.setDividerLocation(0);
      return;
    }

    try
    {
      htmlPane.setPage(url);
      if(!urlField.isItem(url))
        urlField.addURL(url);
      else
      {
        urlField.setSelectedItem(url);
      }
      backMenu.setEnabled(urlField.isBackPage());
      backBt.setEnabled(urlField.isBackPage());

      fwdMenu.setEnabled(urlField.isForwardPage());
      fwdBt.setEnabled(urlField.isForwardPage());
    }
    catch(IOException ioe)
    {
      setCursor(cdone);
      warnUser("Can't follow link to " + url );
    }
    setCursor(cdone);
  }



  /**
  *
  * Method to handle hyper link events.
  * @param event	hyper link event
  *
  */
  public void hyperlinkUpdate(HyperlinkEvent event) 
  {
    if(event.getEventType() == HyperlinkEvent.EventType.ACTIVATED)
    {
      setCursor(cbusy);
      try 
      {
	URL url = event.getURL();
        htmlPane.setPage(url);
        if(!urlField.isItem(url))
          urlField.addURL(url);
        else
	{
          urlField.setSelectedItem(url);
          urlField.setLastIndex(url);
        }
      } 
      catch(IOException ioe) 
      {
        setCursor(cdone);
        warnUser("Can't follow link to " +  
                  event.getDescription() );
      }
      
      setCursor(cdone);
    }
    else if(event.getEventType() == HyperlinkEvent.EventType.ENTERED)
      statusField.setText(event.getDescription());
    else if(event.getEventType() == HyperlinkEvent.EventType.EXITED)
      statusField.setText("");
  }


  /**
  *
  * Display a warning message
  * @param message	message to display
  *
  */
  private void warnUser(String message)
  {
    JOptionPane.showMessageDialog(this, message, "Warning", 
                                  JOptionPane.ERROR_MESSAGE);
  }


  /**
  *
  * Used to draw a Shape.
  *
  */
  public static GeneralPath makeShape(float loc[][])
  {
    GeneralPath path = new GeneralPath(GeneralPath.WIND_NON_ZERO);

    path.moveTo(loc[0][0],loc[0][1]);

    for(int i=1; i<loc.length; i++)
      path.lineTo(loc[i][0],loc[i][1]);

    return path;
  }




  /**
  *
  * Jemboss icon button
  *
  */
  public class JIconButton extends JButton 
  {
    public JIconButton(ImageIcon ii) 
    {
      super(ii);
      setContentAreaFilled(false);
      setBorderPainted(false);
      setFocusPainted(false);
    }
  }

  
  public JTree getWebScape()
  {
    DefaultMutableTreeNode top =
        new DefaultMutableTreeNode("Jemboss WebScape");
    createNodes(top);
    
    final JTree webScape = new JTree(top);
    webScape.setShowsRootHandles(true);
    webScape.expandRow(1);
    webScape.getSelectionModel().setSelectionMode
            (TreeSelectionModel.SINGLE_TREE_SELECTION);
    webScape.addTreeSelectionListener(new TreeSelectionListener()
    {
      public void valueChanged(TreeSelectionEvent e) 
      {
        ClassLoader cl = this.getClass().getClassLoader();
        DefaultMutableTreeNode node = (DefaultMutableTreeNode)
                       webScape.getLastSelectedPathComponent();

        if (node == null) return;
        Object nodeInfo = node.getUserObject();
        if(node.isLeaf()) 
        {
          String loc = null;
          DefaultMutableTreeNode parent = (DefaultMutableTreeNode)node.getParent();
          String category = (String)parent.getUserObject();

          String selectedValue = (String)nodeInfo;
          URL inURL = null;
          if(selectedValue.equals("File Manager"))
            inURL = cl.getResource("resources/filemgr.html");
          else if(selectedValue.equals("About"))
            inURL = cl.getResource("resources/readme.html");
          else if(selectedValue.equals("Alignment Editor"))
            inURL = cl.getResource("resources/readmeAlign.html");
          else if(selectedValue.equals("Sequence List"))
            inURL = cl.getResource("resources/seqList.html");
          else if(selectedValue.equals("Results Manager"))
            inURL = cl.getResource("resources/results.html");
          else if (selectedValue.equals("User Guide"))
            loc = "http://emboss.sourceforge.net/Jemboss/guide.html";
          else if(selectedValue.equals("Home") && category.equals("Jemboss"))
            loc = "http://emboss.sourceforge.net/Jemboss/";
          else if(selectedValue.equals("Home") && category.equals("EMBOSS"))
            loc = "http://emboss.sourceforge.net/";
          else if(selectedValue.equals("Apps") && category.equals("EMBOSS"))
            loc = "http://emboss.sourceforge.net/apps/release/5.0/emboss/apps/";

          if(loc != null) 
          {
            try
            {
              inURL = new URL(loc);
            }
            catch(MalformedURLException me)
            {
              warnUser("Can't follow link to " + loc);
            }
          }

          if(inURL != null)
          {
            try
            {
              htmlPane.setPage(inURL);
              if(!urlField.isItem(inURL))
                urlField.addURL(inURL);
              else
              {
                urlField.setSelectedItem(inURL);
                urlField.setLastIndex(inURL);
              }
            }
            catch(IOException ioe)
            {
              setCursor(cdone);
              warnUser("Can't follow link to " + inURL );
            }
            backMenu.setEnabled(urlField.isBackPage());
            backBt.setEnabled(urlField.isBackPage());
                                                                                
            fwdMenu.setEnabled(urlField.isForwardPage());
            fwdBt.setEnabled(urlField.isForwardPage());
          }
        }
      }
    });

    webScape.setRootVisible(false);
    return webScape;
  }

  private void createNodes(DefaultMutableTreeNode top)
  {
    DefaultMutableTreeNode category = null;
    DefaultMutableTreeNode topic    = null;

    category = new DefaultMutableTreeNode("Jemboss");
    top.add(category);
    
    for(int i=0; i<topics.length; i++)
    {
      topic = new DefaultMutableTreeNode(topics[i]);
      category.add(topic);
    }

    category = new DefaultMutableTreeNode("EMBOSS");
    top.add(category);
                                                                                
    for(int i=0; i<embossTopics.length; i++)
    {
      topic = new DefaultMutableTreeNode(embossTopics[i]);
      category.add(topic);
    }

  }
  
  public static void main(String args[])
  {
    ClassLoader cl = ClassLoader.getSystemClassLoader();
    try
    {
      URL inURL = cl.getResource("resources/seqList.html");
      new Browser(inURL,"resources/seqList.html");
    }
    catch (Exception iex)
    {
      System.out.println("Didn't find resources/seqList.html");
    }
  }

}
