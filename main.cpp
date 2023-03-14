#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define DISK_SIZE 256

void decToBinary(int n, char &c)
{
    c = c & 0;
    // array to store binary number
    int binaryNum[8];

    // counter for binary array
    int i = 0;
    while (n > 0)
    {
        // storing remainder in binary array
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }

    // printing binary array in reverse order
    for (int j = i - 1; j >= 0; j--)
    {
        if (binaryNum[j] == 1)
            c = c | 1u << j;
    }
}

int binaryToDec(char c)
{
    int out = 0;
    for (unsigned int i = 0; i < 8; i++)
    {

        int bit = (c >> i) & 1u; //right shift with 00000001
        if (bit == 1)
        {
            out += pow(2, i); //multyplay by his place
        }
    }
    return out;
}

// #define SYS_CALL
// ============================================================================
class fsInode
{
    int fileSize;
    int block_in_use;

    int *directBlocks;
    int singleInDirect;
    int num_of_direct_blocks;
    int block_size;

public:
    fsInode(int _block_size, int _num_of_direct_blocks)
    {
        fileSize = 0;
        block_in_use = 0;
        block_size = _block_size;
        num_of_direct_blocks = _num_of_direct_blocks;
        directBlocks = new int[num_of_direct_blocks];
        assert(directBlocks);
        for (int i = 0; i < num_of_direct_blocks; i++)
        {
            directBlocks[i] = -1;
        }
        singleInDirect = -1;
    }
    int get_number_of_direct_blocks()
    {
        return num_of_direct_blocks;
    }
    void setFileSize(int len)
    {
        fileSize += len;
    }
    void setSingleInDirect(int index)
    {
        singleInDirect = index;
    }
    int getSingleInDirect()
    {
        return singleInDirect;
    }
    int get_direct_blocks(int i)
    {
        return directBlocks[i];
    }
    //updating the index in the array
    void update_directBlocks(int index)
    {
        int i;
        for (i = 0; i < num_of_direct_blocks; i++)
        { //+1 for the single if needed
            if (directBlocks[i] == -1)
            {
                directBlocks[i] = index;
                return;
            }
        }
    }
    //getting the last index of the array
    int get_last_direct_block()
    {
        int i;
        for (i = 0; i < num_of_direct_blocks - 1; i++)
        {
            if (directBlocks[i + 1] == -1)
            {
                return directBlocks[i];
            }
        }
        return directBlocks[num_of_direct_blocks - 1];
    }

    int getFileSize()
    {
        return fileSize;
    }
    int getBlockInUse()
    {
        return block_in_use;
    }
    //updating number of block
    int setBlockInUse(int a)
    {
        block_in_use += a;
        return block_in_use;
    }

    ~fsInode()
    {
        delete directBlocks;
    }
};

// ============================================================================
class FileDescriptor
{
    pair<string, fsInode *> file;
    bool inUse;

public:
    FileDescriptor(string FileName, fsInode *fsi)
    {
        file.first = FileName;
        file.second = fsi;
        inUse = true;
    }

    string getFileName()
    {
        return file.first;
    }
    string setFileName(string c)
    {
        file.first = c;
    }

    fsInode *getInode()
    {

        return file.second;
    }

    pair<string, fsInode *> getFile()
    {
        return file;
    }

    bool isInUse()
    {
        return (inUse);
    }
    void setInUse(bool _inUse)
    {
        inUse = _inUse;
    }
};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"
// ============================================================================
class fsDisk
{
    FILE *sim_disk_fd;

    bool is_formated;

    // BitVector - "bit" (int) vector, indicate which block in the disk is free
    //              or not.  (i.e. if BitVector[0] == 1 , means that the
    //             first block is occupied.
    int BitVectorSize;
    int *BitVector;

    // Unix directories are lists of association structures,
    // each of which contains one filename and one inode number.
    map<string, fsInode *> MainDir;

    //sum of all file size

    // OpenFileDescriptors --  when you open a file,
    // the operating system creates an entry to represent that file
    // This entry number is the file descriptor.
    vector<FileDescriptor> OpenFileDescriptors;

    int direct_enteris;
    int block_size;

public:
    // ------------------------------------------------------------------------
    fsDisk()
    {
        sim_disk_fd = fopen(DISK_SIM_FILE, "r+");
        assert(sim_disk_fd);
        for (int i = 0; i < DISK_SIZE; i++)
        {
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fwrite("\0", 1, 1, sim_disk_fd);
            assert(ret_val == 1);
        }
        fflush(sim_disk_fd);
    }
    ~fsDisk()
    {
        //deleting all the inode
        for (auto it = begin(OpenFileDescriptors); it != end(OpenFileDescriptors); ++it) //over the map to get the c
        {
            if(it->getFileName() != ""){
                 delete it->getInode();
               // it->getInode()->~fsInode();
            }

        }

        fclose(sim_disk_fd);
        delete[] BitVector;
    }
    //going on the fd disk and getting all the char
    int sum_all_fileSize()
    {
        int sum = 0;
        for (auto it = begin(MainDir); it != end(MainDir); ++it)
        {
            sum += it->second->getFileSize();
        }
        return sum;
    }
    bool isformted(){
        return is_formated;
    }
    //doing cast from file to int
    int get_fd_disk()
    {
        return fileno(sim_disk_fd); //cast fron file to fd
    }

    // ------------------------------------------------------------------------
    void listAll()
    {
        int i = 0;
        for (auto it = begin(OpenFileDescriptors); it != end(OpenFileDescriptors); ++it)
        {
            cout << "index: " << i << ": FileName: " << it->getFileName() << " , isInUse: " << it->isInUse() << endl;
            i++;
        }
        char bufy;
        cout << "Disk content: '";
        for (i = 0; i < DISK_SIZE; i++)
        {
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fread(&bufy, 1, 1, sim_disk_fd);
            cout << bufy;
        }
        cout << "'" << endl;
    }

    // ------------------------------------------------------------------------
    void fsFormat(int blockSize = 4, int direct_Enteris_ = 3)
    {
        direct_enteris = direct_Enteris_;
        block_size = blockSize;
        is_formated = true;
        BitVectorSize = DISK_SIZE / block_size;
        BitVector = new int[BitVectorSize];
        cout << "FORMAT DISK: number of blocks: " << BitVectorSize << endl;
    }

    // ------------------------------------------------------------------------
    int CreateFile(string fileName)
    {
        //check maindir if their a place their
        //First thing check if format
        if (is_formated == false)
        {
            cerr << "Coundnt open a new file disk need to be formated";
            return -1;
        }

        for (auto it = begin(OpenFileDescriptors); it != end(OpenFileDescriptors); ++it)
        {
            if (it->getFileName() == fileName)
            {   
                cerr <<"their a file with this name "<<endl;
                return -1;
            }
        }

        //making new one and entering it to the fileDescriptors
        fsInode *temp = new fsInode(block_size, direct_enteris);
        FileDescriptor fd_new = FileDescriptor(fileName, temp);
        OpenFileDescriptors.push_back(fd_new);


        //making new one and entering it to the MainDir

        int i = 0;
        while (i < BitVectorSize)
        {
            if (BitVector[i] == 0)
            {
                int fd = fd_num_(fileName);
                MainDir.insert(OpenFileDescriptors[fd].getFile());
                return fd;
            }
            i++;
        }

        return -1;
    }
    int fd_num_(string fileName)
    {
        for (auto it = begin(OpenFileDescriptors); it != end(OpenFileDescriptors); ++it)
        {
            if (it->getFileName() == fileName)
            {
                return (it - OpenFileDescriptors.begin());
            }
        }
        return -1;
    }
    // ------------------------------------------------------------------------
    int OpenFile(string fileName)
    {
        int fd_back = fd_num_(fileName);
        //if the file is already open

        if (fd_back == -1)
        {
            cerr << "ERR" << endl;
            return -1;
        }
        if (OpenFileDescriptors[fd_back].isInUse() == true)
        {
            cerr << "File already open" << endl;
            return -1;
        }
        else

        {
            OpenFileDescriptors[fd_back].setInUse(true);
            return fd_back;
        }
    }

    // ------------------------------------------------------------------------
    string CloseFile(int fd)
    {

        if (OpenFileDescriptors.size() < fd)
        {
            //no such file
            return "-1";
        }
        //in case the file is exsist and close
        if (OpenFileDescriptors[fd].isInUse() == false)
        {
            return "-1";
        }
        //in case the file is exsist and we need to close it
        OpenFileDescriptors[fd].setInUse(false);
        return OpenFileDescriptors[fd].getFileName();
    }

    int writeToDirectBlocks(int fd, int fd_disk, int len, char *buf, int num_of_blocks_required, int current_position, bool singleRun)
    {

        int fileSize = OpenFileDescriptors[fd].getInode()->getFileSize();

        int in_the_middle_of_block = fileSize % block_size;
        if (in_the_middle_of_block != 0)
        {
            //here you are in the middle of the block and need to write in that block
            if (singleRun)
            {
                char index_of_inside_single;
                int block_to_go_to = OpenFileDescriptors[fd].getInode()->getSingleInDirect();
                lseek(fd_disk, (block_to_go_to * block_size) + ceil((fileSize / ((double)(block_size))) - direct_enteris) - 1, SEEK_SET);

                int n = read(fd_disk, &index_of_inside_single, 1);
                assert(n);
                int index_after_cast = binaryToDec(index_of_inside_single);
                lseek(fd_disk, (index_after_cast * block_size) + in_the_middle_of_block, SEEK_SET);
            }
            else
            {
                //singel is not exsist

                lseek(fd_disk, (OpenFileDescriptors[fd].getInode()->get_last_direct_block() * block_size) + in_the_middle_of_block, SEEK_SET);
            }

            //deciding how many char you need to write in each itreation -> max 4
            int len_to_write = min(len, block_size - in_the_middle_of_block);
            int temp = write(fd_disk, buf + current_position, len_to_write);
            if (temp == -1)
            {
                cerr << "ERR=Coudlnt write to file " << endl;
                return -1;
            }
            current_position += len_to_write; //update position of the "pointer"
            OpenFileDescriptors[fd].getInode()->setFileSize(len_to_write);
        }

        int i;
        for (i = 0; i < BitVectorSize && num_of_blocks_required > 0; i++)
        {
            if (BitVector[i] == 0)
            {
                BitVector[i] = 1;
                if (singleRun)
                {
                    char c = '0';
                    decToBinary(i, c); //changed int to char by asaf fx
                    int fileSize = OpenFileDescriptors[fd].getInode()->getFileSize();
                    lseek(fd_disk, (OpenFileDescriptors[fd].getInode()->getSingleInDirect() * block_size) + ((fileSize - (block_size * direct_enteris)) / block_size), SEEK_SET);
                    int temp = write(fd_disk, &c, 1); //savie in the singel block on the right place the char .
                    if (temp == -1)
                    {
                        cerr << "ERR=Coudlnt write to file " << endl;
                        return -1;
                    }
                }
                else
                {
                    OpenFileDescriptors[fd].getInode()->update_directBlocks(i);
                }
                OpenFileDescriptors[fd].getInode()->setBlockInUse(1);

                lseek(fd_disk, i * block_size, SEEK_SET);
                int len_to_write = min(abs(len - current_position), block_size); 
                int temp = write(fd_disk, buf + current_position, len_to_write); //buf+ _>go on the buf to the right postion
                if (temp == -1)
                {
                    cerr << "ERR=Coudlnt write to file " << endl;
                    return -1;
                }
                current_position += len_to_write;
                num_of_blocks_required -= 1;
                OpenFileDescriptors[fd].getInode()->setFileSize(len_to_write);
            }
        }
        return current_position;
    }

    // ------------------------------------------------------------------------
    int WriteToFile(int fd, char *buf, int len)
    {
        //format is required for writing
        if (is_formated == false)
        {
            cerr << "Coundnt open a new file disk need to be formated";
            return -1;
        }
        if(OpenFileDescriptors[fd].isInUse() == false){
            cerr<<"File is closed "<<endl;
            return -1;
        }



        int all_file_Size = sum_all_fileSize();
        int blockInUse = OpenFileDescriptors[fd].getInode()->getBlockInUse();
        if (len + all_file_Size > DISK_SIZE)
        {
            cerr << "Coundnt write , Not engoh space ";
            return -1;
        }
        int fileSize = OpenFileDescriptors[fd].getInode()->getFileSize();
        int num_of_chars_required = len;
        if (fileSize % block_size != 0)
        {
            num_of_chars_required -= block_size - (fileSize % block_size);
            num_of_chars_required = max(0, num_of_chars_required);
        }
        int num_of_blocks_required = ceil(num_of_chars_required / double(block_size)); //upcast up 1.2->2
        if (num_of_blocks_required + blockInUse > BitVectorSize)
        {
            cerr << "Coundnt write , Not engoh space ";
            return -1;
        }
        int available_blocks = direct_enteris - blockInUse;
        int fd_disk = fileno(sim_disk_fd); //cast fron file to fd .
        int current_position = 0;

        if (available_blocks >= num_of_blocks_required)
        {
            //blocks are available !

            //MIDDLE WORD
            //going on the bit vector and saving the index to the directblocks array
            current_position = writeToDirectBlocks(fd, fd_disk, len, buf, num_of_blocks_required, current_position, false);


        }
        //need to use single
        else if (available_blocks + block_size >= num_of_blocks_required)
        {

            //going throw the dircet enteris first
            if (available_blocks > 0)
            {
                current_position = writeToDirectBlocks(fd, fd_disk, len, buf, available_blocks, current_position, false);
                num_of_blocks_required -= available_blocks;
            }
            //allocting block for the single
            if (OpenFileDescriptors[fd].getInode()->getSingleInDirect() == -1)
            {
                int k;
                for (k = 0; k < BitVectorSize; k++)
                {
                    if (BitVector[k] == 0)
                    {
                        BitVector[k] = 1;
                        OpenFileDescriptors[fd].getInode()->setSingleInDirect(k);
                        break;
                    }

                    //can use the singal
                }
            }
            current_position = writeToDirectBlocks(fd, fd_disk, len, buf, num_of_blocks_required, current_position, true);
        }
        fflush(sim_disk_fd);
    }
    // ------------------------------------------------------------------------
    int DelFile(string FileName)
    {
        auto it_open = begin(OpenFileDescriptors);
        int fd_disk = get_fd_disk();
        int fd; //geting the index inside the filedescriptor
        for (fd = 0; it_open != end(OpenFileDescriptors); ++it_open)
        {
            if (it_open->getFileName() == FileName)
            {
                break;
            }
            fd++;
        }

        //clearing the bitVector
        int file_Size = OpenFileDescriptors[fd].getInode()->getFileSize();
        auto it = begin(MainDir);
        for (; it != end(MainDir); ++it) //over the map to get the corecct one
        {
            if (it->first == FileName)
            {
                int single_indirect = it->second->getSingleInDirect();
                if (single_indirect != -1) //if single exsist need to clear all blocks inside him
                {
                    int number_of_signal_cells = ceil((file_Size / ((double)(block_size))) - direct_enteris);
                    int k = 0;
                    while (k < number_of_signal_cells)
                    {
                        char index_of_inside_single;
                        lseek(fd_disk, (single_indirect * block_size) + k, SEEK_SET);
                        int n = read(fd_disk, &index_of_inside_single, 1);
                        assert(n);
                        int index_after_cast = binaryToDec(index_of_inside_single);
                        BitVector[index_after_cast] = 0;
                        it->second->setBlockInUse(-1);
                        k++;
                    }

                    //clearing the single himself
                    BitVector[single_indirect] = 0;
                    it->second->setBlockInUse(-1);
                }
                break;
            }
        }
        int i;

        for (i = 0; i <= (it->second->getBlockInUse()); i++)
        {
            int index_of_removal_block = it->second->get_direct_blocks(i);
            BitVector[index_of_removal_block] = 0;
        }
        it->second->setBlockInUse(i+1);
        OpenFileDescriptors[fd].setInUse(false);
                        it_open->setFileName(""); //need to be empty

                delete (it_open->getInode());
        return fd;
    }
    // ------------------------------------------------------------------------
    int ReadFromFile(int fd, char *buf, int len)
    {


        int len_to_return = len;
        int number_of_block_required = ceil(len / (double)block_size);
        //need to check if fd have number of block required.
        int fd_disk = get_fd_disk();
        int aviable_blocks = OpenFileDescriptors[fd].getInode()->get_number_of_direct_blocks();
        int file_Size = OpenFileDescriptors[fd].getInode()->getFileSize();
        int len_to_write;
        int single_indirect = OpenFileDescriptors[fd].getInode()->getSingleInDirect();
        bool single_exsist = false;
        if (isformted()==false){
            cerr<<"Disk havent been formted "<<endl;
            buf[0] = '\0';
            return -1;
        }
        if(OpenFileDescriptors[fd].isInUse() == false){
            cerr<<"File is closed "<<endl;
            buf[0] = '\0';
            return -1;
        }

        if (len > file_Size)
        {
            len=file_Size;

        }

        if (single_indirect != -1)
        {
            single_exsist = true;
        }
        //didnt have single and trying to read more chars then he could
        if (!single_exsist && aviable_blocks < number_of_block_required)
        {
            cerr << "To many char requsted " << endl;
            buf[0] = '\0';
            return -1;
        }

        //if got here he didnt use single
        int i, index_of_direct;
        int how_many_itreations = ceil(len / (double)block_size);
        if (how_many_itreations > 3)
        {
            how_many_itreations = 3;
        }
        for (i = 0; i < how_many_itreations; i++)
        {
            //cutting the len to be by the size of block
            if (len > block_size)
                len_to_write = block_size;
            else
                len_to_write = len;

            index_of_direct = OpenFileDescriptors[fd].getInode()->get_direct_blocks(i);
            lseek(fd_disk, (index_of_direct * block_size), SEEK_SET);

            int n = read(fd_disk, (buf + (i * block_size)), len_to_write);
            assert(n);
            len -= len_to_write;
        }
        int run = 0;
        if (single_exsist)
        {
            int sum_of_single_cells = ceil(file_Size / (double)block_size) - (aviable_blocks);
            int remaining_cells_to_read = ceil(len / (double)block_size);

            if (remaining_cells_to_read > sum_of_single_cells)
            { //trying to read more then he can have
                cerr << "To many char requsted " << endl;
                return -1;
            }

            run = 0;
            while (run < remaining_cells_to_read)
            {
                if (len > block_size)
                    len_to_write = block_size;
                else
                    len_to_write = len;

                char index_of_inside_single;

                int calc = (single_indirect * block_size) + run;
                lseek(fd_disk, calc, SEEK_SET);
                int t = read(fd_disk, &index_of_inside_single, 1);
                assert(t);
                int single_num_after_cast = binaryToDec(index_of_inside_single);

                lseek(fd_disk, (single_num_after_cast * block_size), SEEK_SET);
                int buf_place = ((i * block_size) + (run * block_size));
                int n = read(fd_disk, (buf + buf_place), len_to_write);
                assert(n);
                run++;
            }
        }
        buf[len_to_return] = '\0';
        return 0;
    }
};

int main()
{
    int blockSize;
    int direct_entries;
    string fileName;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
    int _fd;

    fsDisk *fs = new fsDisk();
    int cmd_;
    while (1)
    {
        cin >> cmd_;
        switch (cmd_)
        {
        case 0: // exit
            delete fs;
            exit(0);
            break;

        case 1: // list-file
            fs->listAll();
            break;

        case 2: // format
            cin >> blockSize;
            cin >> direct_entries;
            fs->fsFormat(blockSize, direct_entries);
            break;

        case 3: // create-file
            cin >> fileName;
            _fd = fs->CreateFile(fileName);
            cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

        case 4: // open-file
            cin >> fileName;
            _fd = fs->OpenFile(fileName);
            cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

        case 5: // close-file
            cin >> _fd;
            fileName = fs->CloseFile(_fd);
            cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

        case 6: // write-file
            cin >> _fd;
            cin >> str_to_write;
            fs->WriteToFile(_fd, str_to_write, strlen(str_to_write));

            break;

        case 7: // read-file
            cin >> _fd;
            cin >> size_to_read;
            fs->ReadFromFile(_fd, str_to_read, size_to_read);
            cout << "ReadFromFile: " << str_to_read << endl;
            break;

        case 8: // delete file
            cin >> fileName;
            _fd = fs->DelFile(fileName);
            cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;
        default:
            break;
        }
    }
}