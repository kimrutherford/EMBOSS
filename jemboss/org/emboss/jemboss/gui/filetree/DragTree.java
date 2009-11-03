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
import java.util.Vector;
import java.util.Enumeration;
import java.util.Hashtable;

import org.emboss.jemboss.soap.PrivateRequest;
import org.emboss.jemboss.gui.ShowResultSet;
import org.emboss.jemboss.JembossParams;

/**
*
* Creates a local file tree manager for Jemboss. This acts as a drag 
* source and sink for files.
*
*/
public class DragTree extends JTree implements DragGestureListener,
                 DragSourceListener, DropTargetListener, ActionListener,
                 Autoscroll 
{

  /** jemboss properties */
  private JembossParams mysettings;
  /** root directory */
  private File root;
  /** store of directories that are opened */
  private Vector openNode;
  /** file separator */
  private String fs = new String(System.getProperty("file.separator"));
  /** popup menu */
  private JPopupMenu popup;
  /** busy cursor */
  private Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
  /** done cursor */
  private Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);
  /** AutoScroll margin */
  private static final int AUTOSCROLL_MARGIN = 45;
  /** used by AutoScroll method */
  private Insets autoscrollInsets = new Insets( 0, 0, 0, 0 );

  JMenuItem openMenu;
  JMenuItem renameMenuItem;
  JMenuItem deleteMenuItem;
  
  /**
  *
  * @param rt		root directory
  * @param f		frame
  * @param mysettings 	jemboss properties
  *
  */
  public DragTree(File rt, final JFrame f, final JembossParams mysettings) 
  {
    this.mysettings = mysettings;
    this.root = rt;

    DragSource dragSource = DragSource.getDefaultDragSource();

    dragSource.createDefaultDragGestureRecognizer(
       this,                             // component where drag originates
       DnDConstants.ACTION_COPY_OR_MOVE, // actions
       this);                            // drag gesture recognizer

    setDropTarget(new DropTarget(this,this));
    DefaultTreeModel model = createTreeModel(root);
    setModel(model);
    createTreeModelListener();

    this.getSelectionModel().setSelectionMode
                  (TreeSelectionModel.DISCONTIGUOUS_TREE_SELECTION);

    // Popup menu
    addMouseListener(new PopupListener());
    popup = new JPopupMenu();
    
    JMenuItem menuItem = new JMenuItem("Refresh");
    menuItem.addActionListener(this);
    popup.add(menuItem);
    popup.add(new JSeparator());
//open menu
    openMenu = new JMenu("Open With");
    popup.add(openMenu);
    menuItem = new JMenuItem("Jemboss Alignment Editor");
    menuItem.addActionListener(this);
    openMenu.add(menuItem);
    menuItem = new JMenuItem("Text Editor");
    menuItem.addActionListener(this);
    openMenu.add(menuItem);

    renameMenuItem = new JMenuItem("Rename...");
    renameMenuItem.addActionListener(this);
    popup.add(renameMenuItem);
    menuItem = new JMenuItem("New Folder...");
    menuItem.addActionListener(this);
    popup.add(menuItem);
    deleteMenuItem = new JMenuItem("Delete...");
    deleteMenuItem.addActionListener(this);
    popup.add(deleteMenuItem);
    popup.add(new JSeparator());
    menuItem = new JMenuItem("De-select All");
    menuItem.addActionListener(this);
    popup.add(menuItem);

    //Listen for when a file is selected
    MouseListener mouseListener = new MouseAdapter() 
    {
      public void mouseClicked(MouseEvent me) 
      {
        if(me.getClickCount() == 2 && isFileSelection() &&
           !me.isPopupTrigger()) 
        {
          f.setCursor(cbusy);
          FileNode node = getSelectedNode();
          String selected = node.getFile().getAbsolutePath();
          showFilePane(selected,mysettings);
          f.setCursor(cdone);
        }
      }
    };
    this.addMouseListener(mouseListener);

    addTreeExpansionListener(new TreeExpansionListener()
    {
      public void treeCollapsed(TreeExpansionEvent e){} 
      public void treeExpanded(TreeExpansionEvent e) 
      {
        TreePath path = e.getPath();
        if(path != null) 
        {
          FileNode node = (FileNode)path.getLastPathComponent();

          if(!node.isExplored()) 
          {  
            f.setCursor(cbusy);
            exploreNode(node);
            f.setCursor(cdone);
          }
        }
      }
    });

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
    final FileNode node = getSelectedNode();

    if(source.getText().equals("Refresh")) 
    {
      if(node == null)
        newRoot(root.getAbsolutePath());
      else if(node.isLeaf())
        refresh((FileNode)node.getParent());
      else 
        refresh(node);
      return;
    }

    if(node == null)
    {
      JOptionPane.showMessageDialog(null,"No file selected.",
                        "Warning",
                        JOptionPane.WARNING_MESSAGE);
      return;
    }
    
    final File f = node.getFile();

    if(source.getText().equals("Jemboss Alignment Editor"))
    {
      org.emboss.jemboss.editor.AlignJFrame ajFrame =
               new org.emboss.jemboss.editor.AlignJFrame(f);
      ajFrame.setVisible(true);
    }
    else if(source.getText().equals("Text Editor"))
      showFilePane(f.getAbsolutePath(), mysettings);
    else if(source.getText().equals("New Folder..."))
    {
      String path = null;
      if(node.isLeaf())
        path = f.getParent();
      else
        path = f.getAbsolutePath();

      String inputValue = JOptionPane.showInputDialog(null,
                    "Folder Name","Create New Folder in "+path,
                    JOptionPane.QUESTION_MESSAGE);

      if(inputValue != null && !inputValue.equals("") )
      {
        String fullname = path+fs+inputValue;
        File dir = new File(fullname);
        
        if(dir.exists())
          JOptionPane.showMessageDialog(null, fullname+" already exists!",
                                   "Error", JOptionPane.ERROR_MESSAGE);
        else
        {
          if(dir.mkdir()){
            FileNode child = addObject(inputValue,path,node);
            this.expandPath(new TreePath(child.getPath()));
          }
          else
            JOptionPane.showMessageDialog(null,
                       "Cannot make the folder\n"+fullname,
                       "Error", JOptionPane.ERROR_MESSAGE);     
        }
      }
    }
    else if(source.getText().equals("Delete..."))
    {
      File fn[] = getSelectedFiles();
      String names = "";
      for(int i=0; i<fn.length;i++)
         names = names.concat(fn[i].getAbsolutePath()+"\n");
      int n = JOptionPane.showConfirmDialog(null,
                                 "Delete \n"+names+"?",
                                 "Delete Files",
                                 JOptionPane.YES_NO_OPTION);

      FileNode nodes[] = getSelectedNodes();
      if(n == JOptionPane.YES_OPTION)
        for(int i=0; i<nodes.length;i++)
          deleteFile(nodes[i]);
    }
    else if(source.getText().equals("De-select All"))
      clearSelection();
    else if(source.getText().equals("Rename..."))
    {
      String inputValue = (String)JOptionPane.showInputDialog(null,
                              "New "+(isFileSelection()?"File":"Folder")+" Name","Rename "+f.getName(), 
                              JOptionPane.QUESTION_MESSAGE,null,null,f.getName());

      if(inputValue != null && !inputValue.equals("") )
      {
        String path = f.getParent();
        String fullname   = path+fs+inputValue;
        File newFile = new File(fullname);

        try
        {
          renameFile(f,node,newFile.getCanonicalPath());
        }
        catch(IOException ioe){}
      }
    }
  }


  /**
  *
  * Delete a file from the tree
  * @param node		node to delete
  *
  */
  public void deleteFile(final FileNode node)
  {
    File f = node.getFile();
    if (f.isDirectory()){
    	File[] l = f.listFiles();
    	for (int i=0; i<l.length; i++){
    		l[i].delete();
    	}
    }
        
    if(f.delete())
    {
      Runnable deleteFileFromTree = new Runnable()
      {
        public void run () { deleteObject(node); };
      };
      SwingUtilities.invokeLater(deleteFileFromTree);
    }
    else
      JOptionPane.showMessageDialog(null,"Cannot delete\n"+
                         f.getAbsolutePath(),"Warning",
                         JOptionPane.ERROR_MESSAGE);
  }
 
 
  /**
  *
  * Method to rename a file and update the filenode's.
  * @param oldFile 	file to rename
  * @param oldNode 	filenode to be removed
  * @param newFullName 	name of the new file
  *
  */
  private void renameFile(final File oldFile, final FileNode oldNode, 
                          String newFullName)
  {
    final File fnew = new File(newFullName);
    if(fnew.exists())
      JOptionPane.showMessageDialog(null, newFullName+" already exists!",
                               "Warning", JOptionPane.ERROR_MESSAGE);
    else
    {
      if(oldFile.renameTo(fnew))
      {
        Runnable renameFileInTree = new Runnable()
        {
          public void run ()
          {
            addObject(fnew.getName(),fnew.getParent(),oldNode);
	    deleteObject(oldNode);
          };
        };
        SwingUtilities.invokeLater(renameFileInTree);
      }
      else
        JOptionPane.showMessageDialog(null, 
                   "Cannot rename \n"+oldFile.getAbsolutePath()+
                   "\nto\n"+fnew.getAbsolutePath(), "Rename Error",
                   JOptionPane.ERROR_MESSAGE);
    }
    return;
  }


  /**
  * 
  * Define a directory root for the file tree
  * @param newRoot 	directory to use as the root for
  *        		the tree.
  *
  */
  public void newRoot(String newRoot)
  {
    root = new File(newRoot);
    DefaultTreeModel model = (DefaultTreeModel)getModel();
    model = createTreeModel(root);
    setModel(model);
  }


  /**
  *
  * Refresh
  * @param FileNode node to refresh 
  *
  */
  public void refresh(FileNode node)
  {
    node.reExplore();
    DefaultTreeModel model = (DefaultTreeModel)getModel();
    model.nodeStructureChanged(node);
  }


  /**
  *
  * Get FileNode of selected node
  * @return     node that is currently selected
  *
  */
  public FileNode getSelectedNode()
  {
    TreePath path = getLeadSelectionPath();
    if(path == null)
      return null;
    FileNode node = (FileNode)path.getLastPathComponent();
    return node;
  }


  /**
  *
  * Get FileNodes of selected nodes
  * @return     node that is currently selected
  *
  */
  public FileNode[] getSelectedNodes()
  {
    TreePath path[] = getSelectionPaths();
    if(path == null)
      return null;

    int numberSelected = path.length;
    FileNode nodes[] = new FileNode[numberSelected];
    for(int i=0;i<numberSelected;i++)
       nodes[i] = (FileNode)path[i].getLastPathComponent();

    return nodes;
  }


  /**
  *
  * Get selected files
  * @return     node that is currently selected
  *
  */
  public File[] getSelectedFiles()
  {
    FileNode[] fn = getSelectedNodes();
    int numberSelected = fn.length;
    File files[] = new File[numberSelected];
    for(int i=0;i<numberSelected;i++)
       files[i] = fn[i].getFile();

    return files;
  }


  /**
  *
  * Return true if selected node is a file
  * @return true is a file is selected, false if
  *         a directory is selected
  *
  */
  public boolean isFileSelection()
  {
    TreePath path = getLeadSelectionPath();
    if(path == null)
      return false;

    FileNode node = (FileNode)path.getLastPathComponent();
    return node.isLeaf();
  }


  /**
  *
  * Make the given directory the root and create a new
  * DefaultTreeModel.
  * @param root         root directory
  * @param              tree model with the root node set
  *                     to the given directory
  *
  */
  private DefaultTreeModel createTreeModel(File root)
  {
    FileNode rootNode = new FileNode(root);
    rootNode.explore();
    openNode = new Vector();
    openNode.add(rootNode);
    return new DefaultTreeModel(rootNode);
  }


  /**
  *
  * Adding a file (or directory) to the file tree manager.
  * This looks to see if the directory has already been opened
  * and updates the filetree if it has.
  * @param child        new child to add in
  * @param path         path to where child is to be added
  * @param node         node to add child to
  *
  */
  public FileNode addObject(String child,
                            String path, FileNode node)
  {

    DefaultTreeModel model = (DefaultTreeModel)getModel();
    if(node == null)
    {
      node = getNode(path);
      if(node==null)
        return null;
    }

    FileNode parentNode = getNode(path);
    File newleaf = new File(path + fs + child);
    FileNode childNode = null;

    if(parentNode.isExplored())
    {
      childNode = new FileNode(newleaf);
      int index = getAnIndex(parentNode,child);
      if(index > -1)
        model.insertNodeInto(childNode, parentNode, index);
    }
    else if(parentNode.isDirectory())
    {
      exploreNode(parentNode);
      childNode = getNode(path + fs + child);
    }

    // Make sure the user can see the new node.
    this.scrollPathToVisible(new TreePath(childNode.getPath()));
    return childNode;
  }


  /**
  *
  * Delete a node from the JTree
  * @param node         node for deletion
  *
  */
  public void deleteObject(FileNode node)
  {
    DefaultTreeModel model =(DefaultTreeModel)getModel();
    model.removeNodeFromParent(node);
  }


  /**
  *
  * Explore a directory node
  * @param dirNode      direcory node to display
  *
  */
  public void exploreNode(FileNode dirNode)
  {
    DefaultTreeModel model = (DefaultTreeModel)getModel();
    dirNode.explore();
    openNode.add(dirNode);
    model.nodeStructureChanged(dirNode);
  }

  /**
  *
  * Gets the node from the existing explored nodes and their
  * children.
  * @param path         path to a file or directory
  * @return             corresponding node if the directory or
  *                     file is visible otherwise returns null.
  *
  */
  private FileNode getNode(String path)
  {
    Enumeration en = openNode.elements();

    while(en.hasMoreElements())
    {
      FileNode node = (FileNode)en.nextElement();
      String nodeName = node.getFile().getAbsolutePath();
      if(nodeName.equals(path))
        return node;
    }

// check children of explored nodes
    en = openNode.elements();
    while(en.hasMoreElements())
    {
      FileNode child = getChildNode((FileNode)en.nextElement(),path);
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
  private FileNode getChildNode(FileNode parent, String childName)
  {
    for(Enumeration children = parent.children(); children.hasMoreElements() ;)
    {
      FileNode childNode = (FileNode)children.nextElement();
      String nodeName = childNode.getFile().getAbsolutePath();
      if(childName.equals(nodeName))
        return childNode;
    }

    return null;
  }

  /**
  *
  * Finds a new index for adding a new file to the file manager.
  * @param parentNode   parent directory node
  * @param child        new child node
  * @return             index of the child in the directory
  *
  */
  private int getAnIndex(FileNode parentNode, String child)
  {
    //find the index for the child
    int num = parentNode.getChildCount();
    int childIndex = num;
    for(int i=0;i<num;i++)
    {
      String nodeName =
            ((FileNode)parentNode.getChildAt(i)).getFile().getName();
      if(nodeName.compareTo(child) > 0)
      {
        childIndex = i;
        break;
      }
      else if (nodeName.compareTo(child) == 0)  //file already exists
      {
        childIndex = -1;
        break;
      }
    }
    return childIndex;
  }


  /**
  *
  * Read a file into a byte array.
  * @param filename     file name
  * @return             byte[] contents of file
  *
  */
  protected static byte[] readByteFile(String filename)
  {

    File fn = new File(filename);
    byte[] b = null;
    try
    {
      long s = fn.length();
      if(s == 0)
        return b;
      b = new byte[(int)s];
      FileInputStream fi = new FileInputStream(fn);
      fi.read(b);
      fi.close();
    }
    catch (IOException ioe)
    {
      System.out.println("Cannot read file: " + filename);
    }
    return b;

  }

  /**
  *
  * Opens a JFrame with the file contents displayed.
  * @param filename     file name to display
  * @param mysettings   jemboss properties
  *
  */
  public static void showFilePane(String filename, JembossParams mysettings)
  {
    Hashtable contents = new Hashtable();
    contents.put(filename,readByteFile(filename));
    ShowResultSet srs = new ShowResultSet(contents,mysettings);
    srs.setTitle("Local File");
  }


////////////////////
// DRAG AND DROP
////////////////////
// drag source
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
  public void dragDropEnd(DragSourceDropEvent e) {}
  public void dragEnter(DragSourceDragEvent e) {}
  public void dragExit(DragSourceEvent e) {}
  public void dragOver(DragSourceDragEvent e) {}
  public void dropActionChanged(DragSourceDragEvent e) {}

// drop sink
  public void dragEnter(DropTargetDragEvent e)
  {
    if(e.isDataFlavorSupported(RemoteFileNode.REMOTEFILENODE) ||
       e.isDataFlavorSupported(FileNode.FILENODE) )
      e.acceptDrag(DnDConstants.ACTION_COPY_OR_MOVE);
  }

  public void drop(DropTargetDropEvent e)
  {

    Transferable t = e.getTransferable();
    final FileNode dropNode = getSelectedNode();
    if(dropNode == null)
    {
      e.rejectDrop();
      return;
    }

    //local drop
    if(t.isDataFlavorSupported(FileNode.FILENODE))
    {
       try
       {
         FileNode fn = (FileNode)t.getTransferData(FileNode.FILENODE);
         fn = getNode(fn.getFile().getAbsolutePath());

         if (dropNode.isLeaf())
         {
           e.rejectDrop();
           return;
         }
        
         String dropDir = dropNode.getFile().getAbsolutePath();
         String newFullName = dropDir+fs+fn.toString();
         renameFile(fn.getFile(),fn,newFullName);  
       }
       catch(Exception ufe){}        
    }
    //remote drop
    else if(t.isDataFlavorSupported(RemoteFileNode.REMOTEFILENODE))
    {
      try
      {
        final RemoteFileNode fn = 
          (RemoteFileNode)t.getTransferData(RemoteFileNode.REMOTEFILENODE);
        File dropDest = null;
        String dropDir = null;
        if (dropNode.isLeaf()) 
        {
          FileNode pn = (FileNode)dropNode.getParent();
          dropDir = pn.getFile().getAbsolutePath();
          dropDest = new File(dropDir,fn.getFile());
        } 
        else 
        {
          dropDir = dropNode.getFile().getAbsolutePath();
          dropDest = new File(dropDir,fn.getFile());
        }
        try
        {
          setCursor(cbusy);
                                //check we want to & can save
          FileSave fsave = new FileSave(dropDest); 
          if(fsave.doWrite())
          {
            Vector params = new Vector();
            params.addElement("fileroot=" + fn.getRootDir());
            params.addElement(fn.getFullName());
            PrivateRequest gReq = new PrivateRequest(mysettings,
                                "EmbreoFile","get_file",params);

            fsave.fileSaving(gReq.getHash().get("contents"));

            if(fsave.writeOK() && !fsave.fileExists())
            {
              final String ndropDir = dropDir;
              Runnable updateTheTree = new Runnable() 
              {
                public void run () 
                { 
                  addObject(fn.getFile(),ndropDir,dropNode); 
                };
              };
              SwingUtilities.invokeLater(updateTheTree);
            }
          }
          setCursor(cdone);
        } 
        catch (Exception exp) 
        {
          setCursor(cdone);
          System.out.println("DragTree: caught exception");
        }
        e.getDropTargetContext().dropComplete(true);   
      }
      catch (Exception exp)
      {
        e.rejectDrop();
        return;
      }
    }
    else
    {
      e.rejectDrop();
      return;
    }

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
    if (e.isDataFlavorSupported(RemoteFileNode.REMOTEFILENODE))
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
    else if(e.isDataFlavorSupported(FileNode.FILENODE))
    {
      Point ploc = e.getLocation();
      TreePath ePath = getPathForLocation(ploc.x,ploc.y);
      if (ePath == null)
      {
        e.rejectDrag();
        return;
      }
      FileNode node = (FileNode)ePath.getLastPathComponent();
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
    
    return;
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
        FileNode node = getSelectedNode();
        if(node !=null && e.isPopupTrigger()){
            if (node.getFile() == root){
                renameMenuItem.setEnabled(false);
                deleteMenuItem.setEnabled(false);
                openMenu.setEnabled(false);
            } else{
                renameMenuItem.setEnabled(true);
                deleteMenuItem.setEnabled(true);
                if(!node.isDirectory())
                    openMenu.setEnabled(true);
                else
                    openMenu.setEnabled(false);
            }
            popup.show(e.getComponent(),
                    e.getX(), e.getY());
        }
    }
  }

}

