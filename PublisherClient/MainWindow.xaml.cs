///////////////////////////////////////////////////////////////////////
// MainWindow.xaml.cs - GUI for Publisher                            //
// ver 1.0                                                           //
// Yisheng Pang, CSE687 - Object Oriented Design, Spring 2019        //
///////////////////////////////////////////////////////////////////////
/*
 * Package Operations:
 * -------------------
 * This package provides a WPF-based GUI for proj4 - code publisher.  It's 
 * responsibilities are to:
 * - Provide a display of directory contents of a remote ServerPrototype.
 * - It provides a subdirectory list and a filelist for the selected directory.
 * - You can navigate into subdirectories by double-clicking on subdirectory
 *   or the parent directory, indicated by the name "..".
 *   
 * Required Files:
 * ---------------
 * Mainwindow.xaml
 * Translater.dll
 * Translator.dll
 * 
 * Maintenance History:
 * --------------------
 * ver 1.0 : 22 Apr 2019
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.IO;
using System.Text.RegularExpressions;
using MsgPassingCommunication;
using System.Threading;


namespace PublisherClient
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        public string path { get; set; }

        //----< keep track of the selectedfiles >-----------------------
        private List<string> selectedFiles { get; set; } = new List<string>();

        //----< unselecting lock >-------------------
        private bool unselecting = false;

        //----< substance of TranslatoryFactory, using for calling the native C++ interface >
        private ITranslator tran = TranslatorFactory.createTranslator();

        //----< keep track of the html file in the display tab >-----------
        private string selectedHtmlFile = "";

        //----< string of the regex pattern >------------
        private string regex_pattern = "";

        //----< const string of cpp or header filter regex >-------
        private const string cpp_regex = ".*cpp$|.*h$";

        //----< 
        private Boolean isFiltered = false;

        internal string serverName = "localhost";
        internal int serverPort = 8080;
        internal CsEndPoint serverEndPoint_ = new CsEndPoint();
        internal CsEndPoint clientEndPoint = new CsEndPoint();
        //serverEndPoint_.machineAddress = serverName;
        //serverEndPoint_.port = serverPort;

        internal Stack<string> pathStack_ = new Stack<string>();
        internal string selectedDir = "";
        internal Translater translater;

        private Thread rcvThrd = null;
        private Dictionary<string, Action<CsMessage>> dispatcher_
          = new Dictionary<string, Action<CsMessage>>();


        public MainWindow()
        {
            Console.Out.WriteLine("Initializing publisher GUI......");
            InitializeComponent();
            set_curPath();
            path = Directory.GetCurrentDirectory();
            path = getAncestorPath(3, path);
            string remote_path = Directory.GetCurrentDirectory();
            remote_path = getAncestorPath(4, remote_path);
            remote_path = remote_path + "\\TestFiles";

            serverEndPoint_.machineAddress = "localhost";
            serverEndPoint_.port = 8080;
            clientEndPoint.machineAddress = "localhost";
            clientEndPoint.port = 8081;

            translater = new Translater();
            translater.listen(clientEndPoint);
            processMessages();

            loadDispatcher();
            autoTest("localhost", remote_path);

            remote_LoadNavTab();

            Console.Out.WriteLine("It's all done. Enjoy it.");
        }

        //----< autotest > --------------------------
        private void autoTest(string server_url, string remote_path)
        {

            Console.WriteLine("\n\n*******************Autotest********************");
            serverEndPoint_.machineAddress = server_url;
            path = remote_path;
            Console.Out.WriteLine("\n  Server url: localhost");
            Console.Out.WriteLine("\n  Remote path: " + remote_path);
            test_remote_setFiles();

        }

        //----< process incoming messages on child thread >----------------

        private void processMessages()
        {
            ThreadStart thrdProc = () => {
                while (true)
                {
                    CsMessage msg = translater.getMessage();
                    try
                    {
                        string msgId = msg.value("command");
                        Console.Out.WriteLine("\n  client getting message \"{0}\"", msgId);
                        if (dispatcher_.ContainsKey(msgId))
                            dispatcher_[msgId].Invoke(msg);
                    }
                    catch (Exception ex)
                    {
                        Console.Write("\n  {0}", ex.Message);
                        msg.show();
                    }
                }
            };
            rcvThrd = new Thread(thrdProc);
            rcvThrd.IsBackground = true;
            rcvThrd.Start();
        }

        //----< add client processing for message with key >---------------

        private void addClientProc(string key, Action<CsMessage> clientProc)
        {
            dispatcher_[key] = clientProc;
        }

        //----< load getDirs processing into dispatcher dictionary >-------

        private void DispatcherLoadGetDirs()
        {
            Action<CsMessage> getDirs = (CsMessage rcvMsg) =>
            {
                Action clrDirs = () =>
                {
                    //NavLocal.clearDirs();
                    //NavRemote.clearDirs();
                    //Depend.clearDirs();
                    Dirs.Items.Clear();
                };
                Dispatcher.Invoke(clrDirs, new Object[] { });
                var enumer = rcvMsg.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    if (key.Contains("dir"))
                    {
                        Action<string> doDir = (string dir) =>
                        {
                            //NavRemote.addDir(dir);
                            //Depend.addDir(dir);
                            Dirs.Items.Add(dir);
                        };
                        Dispatcher.Invoke(doDir, new Object[] { enumer.Current.Value });
                    }
                }
                Action insertUp = () =>
                {
                    //NavRemote.insertParent();
                    //Depend.insertParent();
                    Dirs.Items.Insert(0, "..");
                };
                Dispatcher.Invoke(insertUp, new Object[] { });
            };
            addClientProc("getDirs", getDirs);
        }

        //----< load getFiles processing into dispatcher dictionary >------

        private void DispatcherLoadGetFiles()
        {
            Action<CsMessage> getFiles = (CsMessage rcvMsg) =>
            {
                Action clrFiles = () =>
                {
                    //NavRemote.clearFiles();
                    //Depend.clearFiles();
                    Files.Items.Clear();
                };
                Dispatcher.Invoke(clrFiles, new Object[] { });
                var enumer = rcvMsg.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    if (key.Contains("file"))
                    {
                        Action<string> doFile = (string file) =>
                        {
                            //NavRemote.addFile(file);
                            //Depend.addFile(file);
                            Files.Items.Add(file);
                        };
                        Dispatcher.Invoke(doFile, new Object[] { enumer.Current.Value });
                    }
                }
            };
            addClientProc("getFiles", getFiles);
        }

        private void DispatcherGetPublishFiles()
        {
            Action<CsMessage> getPublishFiles = (CsMessage rcvMsg) =>
            {
                var enumer = rcvMsg.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    if (key.Contains("file"))
                    {
                        Action<string> doPublish = (string file) =>
                        {
                            selectedlist_display.Items.Add(file);
                        };
                        Dispatcher.Invoke(doPublish, new Object[] { enumer.Current.Value });
                    }
                }
            };
            addClientProc("getPublishFiles", getPublishFiles);
        }

        //----< load showFiles processing into dispatcher dictionary >------
        private void DispatcherShowPublishFile()
        {
            Action<CsMessage> showFile = (CsMessage rcvMsg) =>
            {
                var enumer = rcvMsg.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;

                    if (key.Contains("file"))
                    {
                        Action<string> doShow = (string file) =>
                        {
                            path2.Content = file;
                            selectedHtmlFile = file + ".html";
                            //tran.ConvertToHtml(file);
                            string html = tran.getHtml(selectedHtmlFile);
                            display.Text = html;
                        };
                        Dispatcher.Invoke(doShow, new Object[] { enumer.Current.Value });
                    }
                }
            };
            addClientProc("showFile", showFile);
        }

        //----< load browserile processing into dispatcher dictionary >------
        private void DispatcherBrowserPublishFile()
        {
            Action<CsMessage> browserFile = (CsMessage rcvMsg) =>
            {
                var enumer = rcvMsg.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    if (key.Contains("file"))
                    {
                        Action<string> doBrowser = (string file) =>
                        {
                            try
                            {
                                System.Diagnostics.Process.Start(selectedHtmlFile);
                            }
                            catch
                            {
                                Console.Out.WriteLine("\n      Can't open the selected file");
                            }
                        };
                        Dispatcher.Invoke(doBrowser, new Object[] { enumer.Current.Value });
                    }
                }
            };
            addClientProc("browserFile", browserFile);
        }

        //----< load all dispatcher processing >---------------------------

        private void loadDispatcher()
        {
            DispatcherLoadGetDirs();
            DispatcherLoadGetFiles();
            DispatcherGetPublishFiles();
            DispatcherShowPublishFile();
            DispatcherBrowserPublishFile();
        }

        //----< set the current path >-----------------------------------
        private void set_curPath()
        {
            path = Directory.GetCurrentDirectory();
            path = getAncestorPath(4, path);

            //LoadNavTab();
        }

        //----< find parent paths >--------------------------------------

        private string getAncestorPath(int n, string path)
        {
            for (int i = 0; i < n; ++i)
                path = Directory.GetParent(path).FullName;
            return path;
        }

        //----< load the selected listbox >------------------------------

        private void AddSelectedFiles(string file)
        {
            selected_Files.Items.Add(file);
            //selectedlist_display.Items.Add(file);
        }


        //----< file Find Libs tab with subdirectories and files >-------


        private void remote_LoadNavTab()
        {

            remote_setDirs();
            remote_setFiles();
        }

        //----< show all the directories >--------------------------------

        private void setDirs()
        {
            Dirs.Items.Clear();
            paths.Content = path;
            string[] dirs = Directory.GetDirectories(path);
            Dirs.Items.Add("..");
            foreach (string dir in dirs)
            {
                DirectoryInfo di = new DirectoryInfo(dir);
                string name = System.IO.Path.GetDirectoryName(dir);
                Dirs.Items.Add(di.Name);
            }
        }

        private void remote_setDirs()
        {
            Dirs.Items.Clear();
            paths.Content = path;
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint_));
            msg.add("from", CsEndPoint.toString(clientEndPoint));
            msg.add("command", "getDirs");

            msg.add("path", path);
            translater.postMessage(msg);
        }

        //----< show all the files under the dir >--------------------------

        private void setFiles()
        {
            Files.Items.Clear();
            string[] files = Directory.GetFiles(path);
            foreach (string file in files)
            {
                Regex r = new Regex(regex_pattern, RegexOptions.IgnoreCase);
                string name = System.IO.Path.GetFileName(file);
                Match m = r.Match(name);
                if (m.Success)
                {
                    Files.Items.Add(name);
                }
            }
        }
        //----< remotely show all the files under the dir >--------------------------
        private void remote_setFiles()
        {
            Files.Items.Clear();
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint_));
            msg.add("from", CsEndPoint.toString(clientEndPoint));
            msg.add("command", "getFiles");
            msg.add("regex", regex_pattern);

            msg.add("path", path);
            translater.postMessage(msg);
        }

        private void test_remote_setFiles()
        {

            Console.Out.WriteLine("\n  *It contains file:");
            string[] files = Directory.GetFiles(path);
            string htmlFile = "";
            foreach (string file in files)
            {
                string name = System.IO.Path.GetFileName(file);
                Console.Out.WriteLine("\n     " + name);
            }
            Console.Out.WriteLine("\n  *cpp and header files: ");

            foreach (string file in files)
            {

                Regex r = new Regex(cpp_regex, RegexOptions.IgnoreCase);
                string name = System.IO.Path.GetFileName(file);
                Match m = r.Match(name);
                Console.Out.WriteLine("publish " + name);
                if (m.Success)
                {
                    //msg.remove("file");
                    selectedFiles.Add(file);
                    Console.Out.WriteLine("\n     " + name);
                    test_Publish(file);
                }
            }

            //wait for server processing
            Thread.Sleep(1000);
            foreach (string file in files)
            {
                Regex r = new Regex(cpp_regex, RegexOptions.IgnoreCase);
                string name = System.IO.Path.GetFileName(file);
                Match m = r.Match(name);
                if (m.Success)
                {
                    htmlFile = file + ".html";
                    System.Diagnostics.Process.Start(htmlFile);
                }
            }
        }

        private void chkboxChange(object sender, RoutedEventArgs e)
        {
            if (chkbox.IsChecked == true)
            {
                regex_pattern = cpp_regex;
                remote_setFiles();
            }
            else
            {
                regex_pattern = "";
                remote_setFiles();
            }
        }



        //----< load display tab >--------------------------------------

        private void AddDisplayList()
        {
            selectedlist_display.Items.Clear();
            foreach (string file in selectedFiles)
            {
                tran.ConvertToHtml(file);
                selectedlist_display.Items.Add(file);
            }
        }

        private void remote_AddDisplayList()
        {
            selectedlist_display.Items.Clear();
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint_));
            msg.add("from", CsEndPoint.toString(clientEndPoint));
            msg.add("command", "getPublishFiles");
            foreach (string file in selectedFiles)
            {
                msg.remove("file");
                //tran.ConvertToHtml(file);
                msg.add("file", file);
                translater.postMessage(msg);
            }
        }

        //----< move into double-clicked directory, display contents >---

        private void Dirs_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            try
            {
                string selectedDir = Dirs.SelectedItem.ToString();
                if (selectedDir == "..")
                {
                    path = getAncestorPath(1, path);
                }
                else
                {
                    path = System.IO.Path.Combine(path, selectedDir);
                }
                remote_LoadNavTab();
                //LoadNavTab();
            }
            catch
            {
                // just return
            }
        }

        //-----< filter button click event >----------------------------------

        private void Filter_Click(object sender, RoutedEventArgs e)
        {
            regex_pattern = regex.Text;
            if (regex_pattern == "" && !isFiltered)
            {
                Console.Out.WriteLine("please input regex");
                MessageBox.Show("please input regex");
                return;
            }

            try
            {
                isFiltered = true;
                //Regex r = new Regex(regex_pattern, RegexOptions.IgnoreCase);

                remote_setFiles();
            }
            catch
            {
                Console.Out.WriteLine("please input valid regex");
                MessageBox.Show("please input valid regex");

                //regex_pattern = "";
            }
        }

        //----< handle selections in files listbox >---------------------

        private void Files_SelectionChanged(object sender,
            SelectionChangedEventArgs e)
        {
            if (unselecting)
            {
                unselecting = false;
                return;
            }

            if (e.AddedItems.Count == 0)
                return;
            string selStr = e.AddedItems[0].ToString();
            if (isCppOrHeader(selStr))
            {
                selStr = System.IO.Path.Combine(path, selStr);
                if (!selectedFiles.Contains(selStr))
                {
                    selectedFiles.Add(selStr);
                    AddSelectedFiles(selStr);
                }
                else
                    MessageBox.Show("Already selected");
            }
            else
            {
                MessageBox.Show("only cpp or header file");
            }
        }


        //----< handle unselction operation in selected_files list box >----

        private void Files_Unselect(object sender, MouseButtonEventArgs e)
        {
            try
            {
                unselecting = true;
                string remove_file = selected_Files.SelectedItem.ToString();
                selectedFiles.Remove(remove_file);
                selected_Files.Items.Remove(remove_file);
                Console.Out.WriteLine("Remove successfully");
                //selectedlist_display.Items.Remove(remove_file);
            }
            catch
            {
                return;
            }
        }

        //----< handle publish button click event >--------------------------

        private void Publish_Click(object sender, RoutedEventArgs e)
        {
            if (selected_Files.Items.Count <= 0)
                MessageBox.Show("Please select file");
            else
            {
                remote_AddDisplayList();
                selected_Files.Items.Clear();
                Console.Out.WriteLine("Publish successfully");
                MessageBox.Show("Publish successfully");
            }

        }

        private void test_Publish(string file)
        {
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint_));
            msg.add("from", CsEndPoint.toString(clientEndPoint));
            msg.add("command", "getPublishFiles");
            msg.add("file", file);
            translater.postMessage(msg);

        }

        //----< change display tab selected file >----------------------------

        private void DisplaySelectionChange(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count == 0)
                return;
            string file = e.AddedItems[0].ToString();

            path2.Content = file;
            string htmlFile = tran.getHtml(file);
            display.Text = htmlFile;
        }

        private void remote_show(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count == 0)
                return;
            string file = e.AddedItems[0].ToString();
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint_));
            msg.add("from", CsEndPoint.toString(clientEndPoint));
            msg.add("command", "showFile");
            msg.add("file", file);
            translater.postMessage(msg);
        }

        //----< convert the published file to html using native c++ interface>-----------

        private void Convert(object sender, SelectionChangedEventArgs e)
        {

            if (e.AddedItems.Count == 0)
            {
                MessageBox.Show("Please select a file");
                return;
            }
            string file = e.AddedItems[0].ToString();
            tran.ConvertToHtml(file);
            path2.Content = file;
            selectedHtmlFile = file + ".html";
            Display();
        }

        //----< show the source code of html in a textbox >----------------

        private void Display()
        {
            string htmlFile = tran.getHtml(selectedHtmlFile);
            display.Text = htmlFile;

        }

        //----< open a browser using c# process >---------------------------

        private void OpenBrowser(object sender, RoutedEventArgs e)
        {
            if (selectedHtmlFile == "")
                MessageBox.Show("Please select file");
            else
            {
                System.Diagnostics.Process.Start(selectedHtmlFile);
            }
        }

        private void remote_OpenBrowser(object sender, RoutedEventArgs e)
        {
            if (selectedHtmlFile == "")
                MessageBox.Show("Please select file");
            else
            {
                CsMessage msg = new CsMessage();
                msg.add("to", CsEndPoint.toString(serverEndPoint_));
                msg.add("from", CsEndPoint.toString(clientEndPoint));
                msg.add("command", "browserFile");
                msg.add("file", selectedHtmlFile);
                translater.postMessage(msg);
                //System.Diagnostics.Process.Start(selectedHtmlFile);
            }
        }

        private void test_OpenBrowser(string file)
        {
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint_));
            msg.add("from", CsEndPoint.toString(clientEndPoint));
            msg.add("command", "browserFile");
            msg.add("file", file);
            translater.postMessage(msg);
        }

        //----< check if a file is cpp or header >--------------------------

        private Boolean isCppOrHeader(string file_name)
        {
            Regex r = new Regex(cpp_regex, RegexOptions.IgnoreCase);
            Match m = r.Match(file_name);
            if (m.Success)
                return true;
            else
                return false;

        }

    }
}
