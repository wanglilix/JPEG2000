using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace JPEG2000_MI_Interface
{
    enum ControlEnum { Compress, Decompress,Start };
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow( )
        {
            InitializeComponent();
            UIEnableControl(ControlEnum.Start);
        }

        
        public string InputFile { get; set; }
        public string OutputFile { get; set; }
        public string OutputFileFormat { get; set; }
        public string InputFile_JP2 { get; set; }
        public string OutputFile_JP2 { get; set; }

        private void UIEnableControl(ControlEnum control)
        {
            switch(control)
            {
                case ControlEnum.Compress:
                    groupBox_FileFormat.IsEnabled = true;
                    groupBox_FileFormat.Opacity = 1;
                    groupBox_CompressionProfile.IsEnabled = true;
                    groupBox_CompressionProfile.Opacity = 1;
                    groupBox_Progression.IsEnabled = true;
                    groupBox_Progression.Opacity = 1;
                    groupBox_CodeblockSize.IsEnabled = true;
                    groupBox_CodeblockSize.Opacity = 1;
                    groupBox_ResolutionNum.IsEnabled = true;
                    groupBox_ResolutionNum.Opacity = 1;
                    groupBox_CompressionValue.IsEnabled = true;
                    groupBox_CompressionValue.Opacity = 1;
                    ShowRawPicture_Button.IsEnabled = true;
                    Compress_Button.IsEnabled = true;

                    ShowCompress_Button.IsEnabled = false;
                    groupBox_Decompression_RGBProfile.IsEnabled = false;
                    groupBox_Decompression_RGBProfile.Opacity = 0.2;
                    groupBox_ResolutionNum_Decode.IsEnabled = false;
                    groupBox_ResolutionNum_Decode.Opacity = 0.2;
                    Decompress_Button.IsEnabled = false;
                    ShowDecompress_Button.IsEnabled = false;
                    break;
                case ControlEnum.Decompress:
                    groupBox_FileFormat.IsEnabled = false;
                    groupBox_FileFormat.Opacity = 0.2;
                    groupBox_CompressionProfile.IsEnabled = false;
                    groupBox_CompressionProfile.Opacity = 0.2;
                    groupBox_Progression.IsEnabled = false;
                    groupBox_Progression.Opacity = 0.2;
                    groupBox_CodeblockSize.IsEnabled = false;
                    groupBox_CodeblockSize.Opacity = 0.2;
                    groupBox_ResolutionNum.IsEnabled = false;
                    groupBox_ResolutionNum.Opacity = 0.2;
                    groupBox_CompressionValue.IsEnabled = false;
                    groupBox_CompressionValue.Opacity = 0.2;
                    ShowRawPicture_Button.IsEnabled = true;
                    Compress_Button.IsEnabled = false;

                    ShowCompress_Button.IsEnabled = false;
                    groupBox_Decompression_RGBProfile.IsEnabled = true;
                    groupBox_Decompression_RGBProfile.Opacity = 1;
                    groupBox_ResolutionNum_Decode.IsEnabled = true;
                    groupBox_ResolutionNum_Decode.Opacity = 1;
                    Decompress_Button.IsEnabled = true;
                    ShowDecompress_Button.IsEnabled = false;
                    break;
                case ControlEnum.Start:
                    groupBox_FileFormat.IsEnabled = false;
                    groupBox_FileFormat.Opacity = 0.2;
                    groupBox_CompressionProfile.IsEnabled = false;
                    groupBox_CompressionProfile.Opacity = 0.2;
                    groupBox_Progression.IsEnabled = false;
                    groupBox_Progression.Opacity = 0.2;
                    groupBox_CodeblockSize.IsEnabled = false;
                    groupBox_CodeblockSize.Opacity = 0.2;
                    groupBox_ResolutionNum.IsEnabled = false;
                    groupBox_ResolutionNum.Opacity = 0.2;
                    groupBox_CompressionValue.IsEnabled = false;
                    groupBox_CompressionValue.Opacity = 0.2;
                    ShowRawPicture_Button.IsEnabled = false;
                    Compress_Button.IsEnabled = false;

                    ShowCompress_Button.IsEnabled = false;
                    groupBox_Decompression_RGBProfile.IsEnabled = false;
                    groupBox_Decompression_RGBProfile.Opacity = 0.2;
                    groupBox_ResolutionNum_Decode.IsEnabled = false;
                    groupBox_ResolutionNum_Decode.Opacity = 0.2;
                    Decompress_Button.IsEnabled = false;
                    ShowDecompress_Button.IsEnabled = false;
                    break;

            }
        }

        private void OpenFileButton_Click( object sender , RoutedEventArgs e )
        {
            var inputFileDialog = new System.Windows.Forms.OpenFileDialog
            {
                InitialDirectory = @"D:\",
                Filter = "BMP文件|*.bmp|JP2文件|*.jp2|J2K文件|*.j2k|J2C文件|*.j2c|所有文件|*.*",
                RestoreDirectory = true,
                FilterIndex = 1
            };

            if (inputFileDialog.ShowDialog() != System.Windows.Forms.DialogResult.OK) return;

            InputFile = inputFileDialog.FileName;
            FileNameLabel.Content = InputFile;

            if (System.IO.Path.GetExtension(inputFileDialog.FileName) == ".bmp")
            {
                InputFile = inputFileDialog.FileName;
                InputFile_JP2 = "";
                OutputFile = "";
                OutputFile_JP2 = "";
                FileNameLabel.Content = InputFile;

                UIEnableControl(ControlEnum.Compress);
            }
            else
            {
                InputFile_JP2 = inputFileDialog.FileName;
                InputFile = "";
                OutputFile = "";
                OutputFile_JP2 = "";
                FileNameLabel.Content = InputFile_JP2;

                UIEnableControl(ControlEnum.Decompress);
            }
        }

        private static string ChoisePickUp(Panel panel)
        {
            foreach(UIElement child in panel.Children)
            {
                var radioChild = child as RadioButton;
                if ( radioChild?.IsChecked == true )
                    return radioChild.Content.ToString();
            }
            return "";
        }

        private static void ShowPicture(string OutputFile)
        {
            if ( System.IO.File.Exists( OutputFile ) )
                System.Diagnostics.Process.Start( OutputFile );
        }

        private void Compress_Button_Click( object sender , RoutedEventArgs e )
        {
            OutputFileFormat = ChoisePickUp( FileFormatPanel );

            var outputFileDialog = new System.Windows.Forms.SaveFileDialog()
            {
                InitialDirectory = @"D:\",
                Filter = OutputFileFormat + "文件(*." + OutputFileFormat + ")" + "|*." + OutputFileFormat,
                RestoreDirectory = true
            };

            if ( outputFileDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK )
            {
                OutputFile = outputFileDialog.FileName;

                var cmdEncodingExe = new StringBuilder( Environment.CurrentDirectory );
                cmdEncodingExe.Append( @".\JPEG2000_MI_Encoding.exe " );
                var cmd = new StringBuilder( "-i " );
                cmd.Append( InputFile );
                cmd.Append( " -o " );
                cmd.Append( OutputFile );
                cmd.Append( " -OutFor " );
                cmd.Append( OutputFileFormat.ToUpper() );
                cmd.Append( " -r " );
                cmd.Append( CompressionValueLabel.Content.ToString() );
                //cmd.Append( " -q " );
                //cmd.Append( QualityValuetextBox.Text );
                cmd.Append( " -n " );
                cmd.Append( ResolutionLabel.Content.ToString() );
                cmd.Append( " -b " );
                var size = ChoisePickUp( CodeblockSizePanel ).Split( '*' );
                cmd.Append( size[ 0 ] );
                cmd.Append( "," );
                cmd.Append( size[ 1 ] );
                if ( ChoisePickUp( CompressionProfilePanel ) == "有损压缩" )
                {
                    cmd.Append( " -I " );
                }
                cmd.Append( " -p " );
                cmd.Append( ChoisePickUp( ProgressionOrderPanel ) );

                System.Diagnostics.Process.Start( cmdEncodingExe.ToString() , cmd.ToString() );

                ShowCompress_Button.IsEnabled = true;
            }
            
        }

        private void ShowCompress_Button_Click( object sender , RoutedEventArgs e )
        {
            ShowPicture( OutputFile );
        }

        private void ShowRawPicture_Button_Click( object sender , RoutedEventArgs e )
        {
            if (InputFile.Length != 0)
                ShowPicture(InputFile);
            else if (InputFile_JP2.Length != 0)
                ShowPicture(InputFile_JP2);
        }

        private void Decompress_Button_Click(object sender, RoutedEventArgs e)
        {
            //OutputFileFormat = ChoisePickUp(FileFormatPanel);
            OutputFileFormat = "bmp";

            var outputFileDialog = new System.Windows.Forms.SaveFileDialog()
            {
                InitialDirectory = @"D:\",
                Filter = OutputFileFormat + "文件(*." + OutputFileFormat + ")" + "|*." + OutputFileFormat,
                RestoreDirectory = true
            };

            if (outputFileDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                OutputFile_JP2 = outputFileDialog.FileName;

                var cmdEncodingExe = new StringBuilder(Environment.CurrentDirectory);
                cmdEncodingExe.Append(@".\JPEG2000_MI_Decoding.exe ");
                var cmd = new StringBuilder("-i ");
                cmd.Append(InputFile_JP2);
                cmd.Append(" -o ");
                cmd.Append(OutputFile_JP2);
                cmd.Append(" -OutFor ");
                cmd.Append(OutputFileFormat.ToUpper());
                cmd.Append(" -r ");
                cmd.Append(ResolutionLabel_Decode.Content.ToString());
                cmd.Append(" -b ");
                if (ChoisePickUp(DecompressionProfile_RGBPanel) == "强行RGB转换")
                {
                    cmd.Append(" -force-rgb ");
                }
                System.Diagnostics.Process.Start(cmdEncodingExe.ToString(), cmd.ToString());

                ShowDecompress_Button.IsEnabled = true;
            }
        }

        private void ShowDecompress_Button_Click(object sender, RoutedEventArgs e)
        {
            ShowPicture(OutputFile_JP2);
        }
    }
}
