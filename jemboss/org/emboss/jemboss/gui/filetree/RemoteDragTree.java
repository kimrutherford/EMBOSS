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

package org.emboss.jemboss.gui.filetree;

import java.awt.*;
import java.awt.event.*;
import java.awt.datatransfer.*;
import java.awt.dnd.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.tree.*;
import java.io.*;
import java.util.*;

import org.emboss.jemboss.gui.ShowResultSet;
import org.emboss.jemboss.gui.ResultsMenuBar;
import org.emboss.jemboss.soap.*;
import org.emboss.jemboss.JembossParams;

/**
*
* Creates a remote file tree which is a drag source & sink
*
*/
public class RemoteDragTree extends JTree implements DragGestureListener,
                           DragSourceListener, DropTargetListener, ActionListener,
                           Autoscroll 
{

  /** jemboss properties */
  private JembossParams mysettings; 
  /** remote directory roots */
  private static FileRoots froots;
  /** popup menu */
  private JPopupMenu popup;
  /** file separator */
  private String fs = new String(System.getProperty("file.separator"));
  /** line separator */
  private String ls = new String(System.getProperty("line.separator"));
  /** store of directories that are opened */
  private Vector openNode;
  /** busy cursor */
  private Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
  /** done cursor */
  private Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);
  /** AutoScroll margin */
  private static final int AUTOSCROLL_MARGIN = 45;
  /** used by AutoScroll method */
  private Insets autoscrollInsets = new Insets( 0, 0, 0, 0 );


  /**
  *
  * @param mysettings	jemboss properties
  * @param froots	remote directory roots
  *
  */
  public RemoteDragTree(final JembossParams mysettings, FileRoots froots)
  {
    this.mysettings = mysettings;
    this.froots = froots;

    DragSource dragSource = DragSource.getDefaultDragSource();
    dragSource.createDefaultDragGestureRecognizer(
             this,                             // component where drag originates
             DnDConstants.ACTION_COPY_OR_MOVE, // actions
             this);                            // drag gesture recognizer

    setDropTarget(new DropTarget(this,this));
    DefaultTreeModel model = createTreeModel(" ");
    setModel(model);
    createTreeModelListener();

    this.getSelectionModel().setSelectionMode
                  (TreeSelectionModel.DISCONTIGUOUS_TREE_SELECTION);

// popup menu
    addMouseListener(new PopupListener());
    popup = new JPopupMenu();
    JMenuItem menuItem = new JMenuItem("Refresh");
    menuItem.addActionListener(this);
    popup.add(menuItem);
    popup.add(new JSeparator());
//open menu
    JMenu openMenu = new JMenu("Open With");
    popup.add(openMenu);
    menuItem = new JMenuItem("Jemboss Aligmnment Editor");
    menuItem.addActionListener(this);
    openMenu.add(menuItem);
    menuItem = new JMenuItem("Text Editor");
    menuItem.addActionListener(this);
    openMenu.add(menuItem);

    menuItem = new JMenuItem("Rename...");
    menuItem.addActionListener(this);
    popup.add(menuItem);
    menuItem = new JMenuItem("New Folder...");
    menuItem.addActionListener(this);
    popup.add(menuItem);
    menuItem = new JMenuItem("Delete...");
    menuItem.addActionListener(this);
    popup.add(menuItem);
    popup.add(new JSeparator());
    menuItem = new JMenuItem("De-select All");
    menuItem.addActionListener(this);
    popup.add(menuItem);


    //Listen for when a file is selected
    addMouseListener(new MouseListener()
    {
      public void mouseClicked(MouseEvent me)
      {
        if(me.getClickCount() == 2 && isFileSelection())
        {
          setCursor(cbusy);
          RemoteFileNode node = (RemoteFileNode)getLastSelectedPathComponent();
          if(node==null)
            return;
          if(node.isLeaf())
            showFilePane(node.getFullName(),mysettings);
          setCursor(cdone);
        }
      }
      public void mousePressed(MouseEvent me){}
      public void mouseEntered(MouseEvent me){}
      public void mouseExited(MouseEvent me){}
      public void mouseReleased(MouseEvent me){}
    });

    addTreeExpansionListener(new TreeExpansionListener()
    {
      public void treeExpanded(TreeExpansionEvent e) 
      {
        TreePath path = e.getPath();
        if(path != null) 
        {
          setCursor(cbusy);
          RemoteFileNode node = (RemoteFileNode)path.getLastPathComponent();

          if(!node.isExplored()) 
            exploreNode(node);
          setCursor(cdone);
        }
      }
      public void treeCollapsed(TreeExpansionEvent e){}
    });

  }


  /**
  *
  * This is used to refresh the file manager
  *
  */
  public void refreshRoot()
  {
    DefaultTreeModel model = (DefaultTreeModel)getModel();
    model = createTreeModel(" ");
    setModel(model);
  }


  /**
  *
  * Popup menu actions
  * @param e	action event
  *
  */
  public void actionPerformed(ActionEvent e)
  {

    JMenuItem source = (JMenuItem)(e.getSource());
    final RemoteFileNode node = getSelectedNode();
    if(node == null)
    {
      JOptionPane.showMessageDialog(null,"No file selected.",
                                    "Warning",
                                    JOptionPane.WARNING_MESSAGE);
      return;
    }

    final String fn = node.getFullName();
    final String parent = node.getPathName();
    String rootPath = node.getRootDir();
    RemoteFileNode pn = node;

    if(source.getText().equals("Refresh"))
    {
      refreshRoot();
    }
    else if(source.getText().equals("Jemboss Aligmnment Editor"))
    {
      Vector params = new Vector();
      String options= "fileroot=" + froots.getCurrentRoot();
      params.addElement(options);
      params.addElement(fn);
  
      try
      {
        PrivateRequest gReq = new PrivateRequest(mysettings,"EmbreoFile",
                                                    "get_file",params);
        org.emboss.jemboss.editor.AlignJFrame ajFrame =
                      new org.emboss.jemboss.editor.AlignJFrame(
                              (String)gReq.getHash().get("contents"),fn);
        ajFrame.setVisible(true);
      }
      catch(JembossSoapException eae)
      {
        System.out.println("RemoteDragTree :: JembossSoapException "+fn);
      }
    }
    else if(source.getText().equals("Text Editor"))
      showFilePane(fn, mysettings);
    else if(source.getText().equals("New Folder..."))
    {
      final String inputValue = JOptionPane.showInputDialog(null,
                          "Folder Name","Create New Folder in",
                          JOptionPane.QUESTION_MESSAGE);

      String dropDest = null;
    
      if(node.isLeaf())
      {
        pn = (RemoteFileNode)node.getParent();
        dropDest = pn.getFullName() + "/" + inputValue; //assumes unix file sep.!
      }
      else
        dropDest = node.getFullName() + "/" + inputValue;

      String newNode = pn.getServerName();
      if(!newNode.endsWith("/"))
        newNode = newNode.concat("/");
      newNode = newNode.concat(inputValue);

      if(nodeExists(pn,newNode))
        return;

      if(inputValue != null && !inputValue.equals("") )
      {
        final RemoteFileNode pnn = pn;

        Vector params = new Vector();
        params.addElement("fileroot=" + rootPath);
        params.addElement(dropDest);

        try
        {
          setCursor(cbusy);
          PrivateRequest gReq = new PrivateRequest(mysettings,
                                 "EmbreoFile","mkdir",params);
          setCursor(cdone);
          Runnable addDirToTree = new Runnable()
          {
            public void run () { addObject(pnn,inputValue,true); };
          };
          SwingUtilities.invokeLater(addDirToTree);
        }
        catch (JembossSoapException jse)
        {
          setCursor(cdone);
        }
      }
    }
    else if(source.getText().equals("Delete..."))
    {
      RemoteFileNode nodes[] = getSelectedNodes();
      String sname = "";
      for(int i=0;i<nodes.length;i++)
        sname = sname.concat(nodes[i].getServerName()+ls);
 
      int n = JOptionPane.showConfirmDialog(null,
             "Delete"+ls+sname+"?", "Delete "+sname,
             JOptionPane.YES_NO_OPTION);
      if(n == JOptionPane.YES_OPTION)
        for(int i=0;i<nodes.length;i++)
          deleteNode(nodes[i]);
    }
    else if(source.getText().equals("De-select All"))
      clearSelection();
    else if(source.getText().equals("Rename..."))
    {
      if(node.isLeaf())
      {
        String inputValue = (String)JOptionPane.showInputDialog(null,
                              "New File Name","Rename "+fn,
                              JOptionPane.QUESTION_MESSAGE,null,null,fn);

        pn = (RemoteFileNode)node.getParent();

        if(inputValue != null && !inputValue.equals("") )
        {
          String newfile = null;
          if(parent.endsWith("/"))
            newfile = parent+inputValue;
          else
            newfile = parent+"/"+inputValue;
          String dir = ((RemoteFileNode)node.getParent()).getFullName();
          if(inputValue.indexOf("/") > 0)
          {
            int index = inputValue.lastIndexOf("/");
            dir = inputValue.substring(0,index);
          }
      
          RemoteFileNode parentNode = getNode(dir);
          if(!nodeExists(parentNode,newfile))
            rename(rootPath,fn,parent,inputValue,node,parentNode);
        }
      }
    }
  }


  /**
  *
  * Delete a node (file or directory) from the tree 
  * and from the server
  * @param node	node to remove
  *
  */
  private void deleteNode(final RemoteFileNode node)
  {
    setCursor(cbusy);
    String rootPath = node.getRootDir();
    String dropDest = node.getFullName();
    Vector params = new Vector();
    params.addElement("fileroot=" + rootPath);
    params.addElement(dropDest);

    if(node.isLeaf())         // file deletion
    {
      try
      {
        PrivateRequest gReq = new PrivateRequest(mysettings,
                               "EmbreoFile","delFile",params);
        Runnable deleteFileFromTree = new Runnable()
        {
          public void run () { deleteObject(node); };
        };
        SwingUtilities.invokeLater(deleteFileFromTree);
      }
      catch (JembossSoapException jse)
      {
        setCursor(cdone);
      }
    }
    else                      // directory deletion
    {
      try
      {
        FileList efl = new FileList(mysettings,rootPath,dropDest);
        Vector children = efl.fileVector();
        if(children.size() > 0)
          JOptionPane.showMessageDialog(null,"Cannot delete"+ls+
                 node.getServerName()+
                 ls+"this directory is not empty","Warning",
                 JOptionPane.ERROR_MESSAGE);
        else
        {
          PrivateRequest gReq = new PrivateRequest(mysettings,
                               "EmbreoFile","delDir",params);
          Runnable deleteFileFromTree = new Runnable()
          {
            public void run () { deleteObject(node); };
          };
          SwingUtilities.invokeLater(deleteFileFromTree);
        }
      }
      catch (JembossSoapException jse)
      {
        setCursor(cdone);
      }
    }
    setCursor(cdone);
  }


  /**
  *
  * Explore a directory node
  * @param dirNode      direcory node to display
  *
  */
  public void exploreNode(RemoteFileNode dirNode)
  {
    DefaultTreeModel model = (DefaultTreeModel)getModel();
    dirNode.explore();
    openNode.add(dirNode);
    model.nodeStructureChanged(dirNode);
  }


  /**
  *
  * Test if a child node exists
  * @param parentNode	parent node
  * @param child	child to test for
  *
  */
  public boolean nodeExists(RemoteFileNode parentNode,String child)
  {
    RemoteFileNode childNode = getChildNode(parentNode,child);
    if(childNode != null)
    {
      String ls = System.getProperty("line.separator");
      JOptionPane.showMessageDialog(null, child+ls+" already exists!",
                                    "File Exists",
                                    JOptionPane.ERROR_MESSAGE);
      return true;
    }

    return false;
  }


  /**
  *
  * Rename a node from the tree
  * @param rootPath		root path
  * @param fullname		full name of node to rename
  * @param pathToNewFile	path to new file
  * @param newfile		new file name
  * @param node			node to rename
  *
  */
  private void rename(String rootPath, final String fullname, 
                      String pathToNewFile, final String newfile,
                      final RemoteFileNode node, final RemoteFileNode parentNode)
  {
    Vector params = new Vector();
    params.addElement("fileroot=" + rootPath);
    params.addElement(fullname);
    params.addElement(pathToNewFile+"/"+newfile);
    try
    {
      setCursor(cbusy);
      PrivateRequest gReq = new PrivateRequest(mysettings,
                             "EmbreoFile","rename",params);
      setCursor(cdone);
      Runnable deleteFileFromTree = new Runnable()
      {
        public void run ()
        {
          addObject(parentNode,newfile,false);
          deleteObject(node);
        };
      };
      SwingUtilities.invokeLater(deleteFileFromTree);
    }
    catch (JembossSoapException jse)
    {
      setCursor(cdone);
    }
  }


  /**
  *
  * Adding a file (or directory) to the file tree manager.
  * This looks to see if the directory has already been opened
  * and updates the filetree if it has.
  * @param parentNode   parent node
  * @param child        file to add to the tree
  * @param ldir         true if child is a directory
  *
  */
  public void addObject(RemoteFileNode parentNode,String child,
                        boolean ldir)
  {
    DefaultTreeModel model = (DefaultTreeModel)getModel();
    
    if(parentNode == null)
      return;

    String path = parentNode.getFullName();
    //create new file node
    if(path.equals(" "))
      path = "";

    if(child.indexOf("/") > -1)
      child = child.substring(child.lastIndexOf("/")+1);

    RemoteFileNode childNode = null;
    if(!parentNode.isExplored())
    {
      exploreNode(parentNode);
      childNode = getNode(parentNode.getServerName() + "/" + child);
    }
    else
    {
      childNode = new RemoteFileNode(mysettings,froots,
                                     child,null,path,ldir);

      //find the index for the child
      int num = parentNode.getChildCount();
      int childIndex = num;
      for(int i=0;i<num;i++)
      {
        String nodeName = ((RemoteFileNode)parentNode.getChildAt(i)).getFile();
        if(nodeName.compareTo(child) > 0)
        {
          childIndex = i;
          break;
        }
        else if(nodeName.compareTo(child) == 0)  //file already exists
        {
          childIndex = -1;
          break;
        }
      }
      if(childIndex != -1)
        model.insertNodeInto(childNode,parentNode,childIndex);
    }

    // Make sure the user can see the new node.
    this.scrollPathToVisible(new TreePath(childNode.getPath()));

    return;
  }


  /**
  *
  * Delete a node from the tree
  * @param node		node to remove
  *
  */
  public void deleteObject(RemoteFileNode node)
  {
    RemoteFileNode parentNode = (RemoteFileNode)node.getParent();
    DefaultTreeModel model = (DefaultTreeModel)getModel();
    model.removeNodeFromParent(node);
//  model.nodeStructureChanged(parentNode);
    return;
  }


  /**
  *
  * Get the contents of a local file as a byte array
  * @param name         local file name
  * @return             contents of file
  *
  */
  public byte[] getLocalFile(File name)
  {
    byte[] b = null;
    try
    {
      long s = name.length();
      b = new byte[(int)s];
      FileInputStream fi = new FileInputStream(name);
      fi.read(b);
      fi.close();
    }
    catch (IOException ioe)
    {
      System.out.println("Cannot read file: " + name);
    }
    return b;
  }


  /**
  *
  * Get RemoteFileNode of selected node
  * @return     node that is currently selected
  *
  */
  public RemoteFileNode getSelectedNode()
  {
    TreePath path = getLeadSelectionPath();
    if(path == null)
      return null;
    RemoteFileNode node = (RemoteFileNode)path.getLastPathComponent();
    return node;
  }


  /**
  *
  * Get RemoteFileNodes of selected nodes
  * @return     node that is currently selected
  *
  */
  public RemoteFileNode[] getSelectedNodes()
  {
    TreePath path[] = getSelectionPaths();
    if(path == null)
      return null;

    int numberSelected = path.length;
    RemoteFileNode nodes[] = new RemoteFileNode[numberSelected];
    for(int i=0;i<numberSelected;i++)
       nodes[i] = (RemoteFileNode)path[i].getLastPathComponent();

    return nodes;
  }


  /**
  *
  * Determine if selected node is a file
  * @return     true if the selected node is a file
  *
  */
  public boolean isFileSelection()
  {
    TreePath path = getLeadSelectionPath();
    if(path == null)
      return false;
    RemoteFileNode node = (RemoteFileNode)path.getLastPathComponent();
    return !node.isDirectory();
  }


  /**
  *
  * Get the selected node file name
  * @return     file name
  *
  */
  public String getFilename()
  {
    TreePath path = getLeadSelectionPath();
    RemoteFileNode node = (RemoteFileNode)path.getLastPathComponent();
    return node.getServerName();
  }


  /**
  *
  * Creates the tree model for the given root
  * @param root         root to create model for
  * @return             tree model
  *
  */
  private DefaultTreeModel createTreeModel(String root)
  {
    setCursor(cbusy);
    RemoteFileNode rootNode = new RemoteFileNode(mysettings,froots,
                                                   root,null,null);
    rootNode.explore();
    openNode = new Vector();
    openNode.add(rootNode);
    setCursor(cdone);
    return new DefaultTreeModel(rootNode);
  }

  /**
  *
  * Gets the node from the existing explored nodes and their children.
  * @param path         path to a file or directory
  * @return             corresponding node if the directory or
  *                     file is visible otherwise returns null.
  *
  */
  private RemoteFileNode getNode(String path)
  {
    Enumeration en = openNode.elements();

    while(en.hasMoreElements())
    {
      RemoteFileNode node = (RemoteFileNode)en.nextElement();
      String nodeName = node.getFullName();

      if(nodeName.equals(path))
        return node;
    }

// check children of explored nodes
    en = openNode.elements();
    while(en.hasMoreElements())
    {
      RemoteFileNode child = getChildNode((RemoteFileNode)en.nextElement(),path);
      if(child != null)
        return child;
    }

    return null;
  }


  /**
  *
  * Gets the child node of a parent node
  * @param parent       parent node
  * @param childName    name of child
  * @return the child node
  *
  */
  private RemoteFileNode getChildNode(RemoteFileNode parent, String childName)
  {
    for(Enumeration children = parent.children();
                    children.hasMoreElements() ;)
    {
      RemoteFileNode childNode = (RemoteFileNode)children.nextElement();
      String nodeName = childNode.getServerName();
      if(childName.equals(nodeName))
        return childNode;
    }

    return null;
  }


  /**
  *
  * Opens a JFrame with the file contents displayed.
  * @param filename     file name
  * @param mysettings   jemboss properties
  *
  */
  public static void showFilePane(String filename, JembossParams mysettings)
  {
    try
    {
      Vector params = new Vector();
      String options= "fileroot=" + froots.getCurrentRoot();
      params.addElement(options);
      params.addElement(filename);

      PrivateRequest gReq = new PrivateRequest(mysettings,"EmbreoFile",
                                                    "get_file",params);
 
      Hashtable hfile = new Hashtable();
      Object contents = gReq.getHash().get("contents");
      if(contents instanceof String)
        hfile.put(filename, ((String)contents).getBytes() );
      else
        hfile.put(filename, (byte[])contents);
      ShowResultSet srs = new ShowResultSet(hfile,mysettings);
      srs.setTitle("Remote File");
    }
    catch(JembossSoapException eae)
    {
    }

  }


////////////////////
// DRAG AND DROP
////////////////////
  public void dragGestureRecognized(DragGestureEvent e) 
  {
    // ignore if mouse popup trigger
    InputEvent ie = e.getTriggerEvent();
    if(ie instanceof MouseEvent)
      if(((MouseEvent)ie).isPopupTrigger())
        return;

    // drag only files 
    if(isFileSelection())
      e.startDrag(DragSource.DefaultCopyDrop,     // cursor
                 (Transferable)getSelectedNode(), // transferable data
                                       this);     // drag source listener
  }

// Source
  public void dragDropEnd(DragSourceDropEvent e) {}
  public void dragEnter(DragSourceDragEvent e) {}
  public void dragExit(DragSourceEvent e) {}
  public void dragOver(DragSourceDragEvent e) {}
  public void dropActionChanged(DragSourceDragEvent e) {}

// Target
  public void dragEnter(DropTargetDragEvent e)
  {
    if(e.isDataFlavorSupported(FileNode.FILENODE) ||
       e.isDataFlavorSupported(RemoteFileNode.REMOTEFILENODE))
      e.acceptDrag(DnDConstants.ACTION_COPY_OR_MOVE);
  }

  public void drop(DropTargetDropEvent e)
  {
    Transferable t = e.getTransferable();

    if(t.isDataFlavorSupported(RemoteFileNode.REMOTEFILENODE))
    {
      try
      {
        Point ploc = e.getLocation();
        TreePath dropPath = getPathForLocation(ploc.x,ploc.y);
        if(dropPath != null)
        {
          RemoteFileNode fn =
            (RemoteFileNode)t.getTransferData(RemoteFileNode.REMOTEFILENODE);

          fn = getNode(fn.getServerName()); // need to get the instance of
                                            // this directly to manipulate tree
          String dropDest = null;
          RemoteFileNode fdropPath = (RemoteFileNode)dropPath.getLastPathComponent();
          String dropRoot = fdropPath.getRootDir();
          String dropFile = null;
          if(fdropPath.getFile().equals(" "))
            dropFile = fn.getFile();
          else
            dropFile = fdropPath.getFile()+"/"+fn.getFile();

          if(!nodeExists(fdropPath,fdropPath.getServerName()+fn.getFile()))
            rename(fn.getRootDir(),fn.getFullName(),
                   fdropPath.getPathName(),
                   dropFile, fn, fdropPath);
        }
      }
      catch(Exception ex){}
    }
    else if(t.isDataFlavorSupported(FileNode.FILENODE))
    {
      try
      {
        Point ploc = e.getLocation();
        TreePath dropPath = getPathForLocation(ploc.x,ploc.y);
        if (dropPath != null) 
        {
          FileNode fn = (FileNode)t.getTransferData(FileNode.FILENODE);
          File lfn = fn.getFile();

          String dropDest = null;
          RemoteFileNode fdropPath = (RemoteFileNode)dropPath.getLastPathComponent();
          String dropRoot = fdropPath.getRootDir();
 
          RemoteFileNode pn = fdropPath;         
          if(fdropPath.isLeaf())
          {
            pn = (RemoteFileNode)fdropPath.getParent();
            dropDest = pn.getFullName() + "/" + lfn.getName(); //assumes unix file sep.!
          }
          else
            dropDest = fdropPath.getFullName()+ "/" + lfn.getName();

          if(!nodeExists(pn,pn.getServerName()+lfn.getName()))
          {

            try 
            {
              Vector params = new Vector();
              byte[] fileData = getLocalFile(lfn);
              params.addElement("fileroot=" + dropRoot);
              params.addElement(dropDest);
              params.addElement(fileData);

              setCursor(cbusy);
              PrivateRequest gReq = new PrivateRequest(mysettings,"EmbreoFile",
                                                             "put_file",params);
              setCursor(cdone);
              //add file to remote file tree
              RemoteFileNode parentNode = fdropPath;
              if(parentNode.isLeaf())
                parentNode = (RemoteFileNode)fdropPath.getParent();
              else
                parentNode = fdropPath;
           
              if(parentNode.isExplored())
                addObject(parentNode,lfn.getName(),false);
              else
              {
                exploreNode(parentNode);
                RemoteFileNode childNode = getNode(parentNode.getServerName() 
                                                       + "/" + lfn.getName());
                scrollPathToVisible(new TreePath(childNode.getPath())); 
              }
            } 
            catch (Exception exp) 
            {
              setCursor(cdone);
              System.out.println("RemoteDragTree: caught exception " + dropRoot +
                  " Destination: " + dropDest + " Local File " + lfn.toString());
            }
          }
          else
            e.rejectDrop();
        }
      }
      catch (Exception ex) 
      {
      }
    } 
    else
      e.rejectDrop();

  }

  /**
  *
  * When a suitable DataFlavor is offered over a remote file
  * node the node is highlighted/selected and the drag
  * accepted. Otherwise the drag is rejected.
  *
  */
  public void dragOver(DropTargetDragEvent e)
  {
    if (e.isDataFlavorSupported(FileNode.FILENODE))
    {
      Point ploc = e.getLocation();
      TreePath ePath = getPathForLocation(ploc.x,ploc.y);
      if (ePath == null)
        e.rejectDrag();
      else
      {
        setSelectionPath(ePath);
        e.acceptDrag(DnDConstants.ACTION_COPY_OR_MOVE);
      }
    }
    else if(e.isDataFlavorSupported(RemoteFileNode.REMOTEFILENODE))
    {
      Point ploc = e.getLocation();
      TreePath ePath = getPathForLocation(ploc.x,ploc.y);
      if (ePath == null)
      {
        e.rejectDrag();
        return;
      }
      RemoteFileNode node = (RemoteFileNode)ePath.getLastPathComponent();
      if(!node.isDirectory())
        e.rejectDrag();
      else
      {
        setSelectionPath(ePath);
        e.acceptDrag(DnDConstants.ACTION_COPY_OR_MOVE);
      }
    }
    else
      e.rejectDrag();

  }

  public void dropActionChanged(DropTargetDragEvent e) {}
  public void dragExit(DropTargetEvent e){}


////////////////////
// AUTO SCROLLING //
////////////////////
  /**
  *
  * Handles the auto scrolling of the JTree.
  * @param location The location of the mouse.
  *
  */
  public void autoscroll( Point location )
  {
    int top = 0, left = 0, bottom = 0, right = 0;
    Dimension size = getSize();
    Rectangle rect = getVisibleRect();
    int bottomEdge = rect.y + rect.height;
    int rightEdge = rect.x + rect.width;
    if( location.y - rect.y < AUTOSCROLL_MARGIN && rect.y > 0 )
      top = AUTOSCROLL_MARGIN;
    if( location.x - rect.x < AUTOSCROLL_MARGIN && rect.x > 0 )
      left = AUTOSCROLL_MARGIN;
    if( bottomEdge - location.y < AUTOSCROLL_MARGIN && bottomEdge < size.height )
      bottom = AUTOSCROLL_MARGIN;
    if( rightEdge - location.x < AUTOSCROLL_MARGIN && rightEdge < size.width )
      right = AUTOSCROLL_MARGIN;
    rect.x += right - left;
    rect.y += bottom - top;
    scrollRectToVisible( rect );
  }


  /**
  *
  * Gets the insets used for the autoscroll.
  * @return The insets.
  *
  */
  public Insets getAutoscrollInsets()
  {
    Dimension size = getSize();
    Rectangle rect = getVisibleRect();
    autoscrollInsets.top = rect.y + AUTOSCROLL_MARGIN;
    autoscrollInsets.left = rect.x + AUTOSCROLL_MARGIN;
    autoscrollInsets.bottom = size.height - (rect.y+rect.height) + AUTOSCROLL_MARGIN;
    autoscrollInsets.right  = size.width - (rect.x+rect.width) + AUTOSCROLL_MARGIN;
    return autoscrollInsets;
  }

  /**
  *
  * Popup menu listener
  *
  */
  class PopupListener extends MouseAdapter
  {
    public void mousePressed(MouseEvent e)
    {
      maybeShowPopup(e);
    }

    public void mouseReleased(MouseEvent e)
    {
      maybeShowPopup(e);
    }

    private void maybeShowPopup(MouseEvent e)
    {
      if(e.isPopupTrigger())
        popup.show(e.getComponent(),
                e.getX(), e.getY());
    }
  }

}
