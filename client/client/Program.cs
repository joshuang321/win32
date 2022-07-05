using System;
using System.Runtime.InteropServices;

namespace client
{
    [StructLayout(LayoutKind.Sequential, Pack = 8)]
    internal struct SERVERINSTANCE
    {
        public IntPtr hOutputPipe,
            hInputPipe;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct PROCESS_INFORMATION
    {
        public IntPtr hProcess;
        public IntPtr hThread;
        public int dwProcessId,
            dwThreadId;
    }

    internal class Program
    {
        [DllImport("serverclient.dll")]
        static internal extern int InstantiateServer(ref SERVERINSTANCE pServInstance,
            ref PROCESS_INFORMATION pprocInfo);

        [DllImport("serverclient.dll")]
        static internal extern void WriteMessage(ref SERVERINSTANCE pServInstance,
            [MarshalAs(UnmanagedType.LPWStr)] string str, int stringLength);

        [DllImport("serverclient.dll")]
        static internal extern void TerminateServer(ref SERVERINSTANCE pServInstance);

        static void Main(string[] args)
        {
            Console.WriteLine("Hello World!");
            SERVERINSTANCE servInstance = new SERVERINSTANCE();
            PROCESS_INFORMATION procInfo = new PROCESS_INFORMATION();

            if (1 == InstantiateServer(ref servInstance, ref procInfo))
            {
                while (true)
                {
                    Console.Write("Input: ");
                    string cmdLineInput = Console.ReadLine();
                    if ("quit" == cmdLineInput)
                    {
                        TerminateServer(ref servInstance);
                        break;
                    }
                    else
                        WriteMessage(ref servInstance,
                            cmdLineInput,
                            cmdLineInput.Length);
                }
            }
        }
    }
}
