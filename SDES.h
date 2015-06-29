#include <cstdio>
#include <vector>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <ctime>
using namespace std;

///debugging
struct debugger{template<typename T> debugger& operator,(const T& v){cerr<<v<<" ";return *this;}}dbg;
#define db(args...) do {cerr << #args << ": "; dbg,args; cerr << endl;} while(0)

/***** SDES_Project *****/
namespace SDES_Project{

    typedef bool bit;
    typedef vector<bit>    bitArray;
    typedef unsigned char byte;
    #define length(x) ((int)(x).size())
    #define setsize(x) resize(x)

    class SDES{

    private:
        /** S-Boxes **/
        int S0[4][4]= {
            {1,0,3,2},
            {3,2,1,0},
            {0,2,1,3},
            {3,1,3,2}
        };
        int S1[4][4]={
            {0,1,2,3},
            {2,0,1,3},
            {3,0,1,0},
            {2,1,0,3}
        };
        bitArray k1, k2;    /** Subkeys **/


        /** Permutations **/
        int P10[10] = {3,5,2,7,4,10,1,9,8,6};
        int P8[8] = {6,3,7,4,8,5,10,9};
        int P4[4] = {2,4,3,1};
        int IP[8] = {2,6,3,1,4,8,5,7};
        int IP_inverse[8] = {4,1,3,5,7,2,8,6};
        int EP8[8] = {4,1,2,3,2,3,4,1};

        /***************************** Utility Functions ***************************/
        bitArray permute(bitArray b, int* p){
            bitArray res(length(b));
            for(int i = 0; i < length(b); ++i)
                res[i] = b[p[i]-1];
            return res;
        }

        bitArray expand_permute(bitArray b, int* p){
            int len = length(b);
            bitArray res(len*2);
            for(int i = 0; i < (2*len); ++i){
                res[i] = b[p[i]-1];
            }
            return res;
        }

        bitArray XOR(bitArray b1, bitArray b2){
            int l = length(b1);
            bitArray res(l);
            for(int i = 0; i < l; ++i)
                res[i] = b1[i] xor b2[i];
            return  res;
        }

        bitArray split(bitArray b, int half){
            bitArray res(length(b)/2);
            for(int i = (half == 1 ? 0 : (length(b)/2)), cnt = 0;      cnt < length(b)/2;   ++cnt, ++i){
                res[cnt] = b[i];
            }
            return res;
        }

        bitArray circular_left_shift(bitArray b, int shift_bits){
            int len = length(b);
            bitArray res(len);
            for(int i = 0, j = shift_bits; i < len; ++i, j = (j+1)%len){
                res[i] = b[j];
            }
            return res;
        }

        bitArray join(bitArray b1, bitArray b2){
            int l1 = length(b1), l2 = length(b2), l = l1+l2;
            bitArray res(l);
            for(int i = 0; i < l; ++i){
                if(i < l1)
                    res[i] = b1[i];
                else
                    res[i] = b2[i-l1];
            }
            return res;
        }

        string bin2str(bit b){
break            return (b == 1 ? "1": "0");
        }

        int binstr2dec(string bstr){
            int res = 0;
            for(int i = length(bstr) - 1, power2 = 1; i >= 0; --i, power2 <<= 1)
                res += power2 * (bstr[i] == '1' ? 1 : 0);
            return res;
        }

        bitArray dec2bin(int dec){
            bitArray res;
            while(dec){
                if(dec&1)
                    res.push_back(1);
                else
                    res.push_back(0);
                dec >>= 1;
            }
            reverse(res.begin(), res.end());
            return res;
        }

        bitArray get_sbox_bits(int box, bitArray b){
            bitArray res;
            int row = binstr2dec(bin2str(b[0]) + bin2str(b[3])),
                col = binstr2dec(bin2str(b[1]) + bin2str(b[2]));
            int x = (box == 0 ? S0[row][col] : S1[row][col]);
            res = dec2bin(x);
            stuff_bits(res, 2);
            return res;
        }

        void stuff_bits(bitArray& b, int l){
            bitArray prefix(l - length(b), 0);
            b.insert(b.begin(), prefix.begin(), prefix.end());
        }

	string to_string(char *m){
	    string res = "";
	    for(int i = 0; i < strlen(m); ++i)
		res += m[i];
	    return res;
	}

	void to_charArray(char* res, string m){
	    for(int i = 0; i < length(m); ++i)
		res[i] = m[i];
	}


        /***************************** Encryption & Decryption Functions *************************/

        void generate_keys(bitArray master_key){
            bitArray K = permute(master_key, P10);
            bitArray b1 = split(K, 1),
                     b2 = split(K, 2);
            k1 = permute(join(circular_left_shift(b1, 1), circular_left_shift(b2, 1)), P8);
            k2 = permute(join(circular_left_shift(b1, 3), circular_left_shift(b2, 3)), P8);
        }

        bitArray F(bitArray b, bitArray sk){
            bitArray p = XOR(expand_permute(b, EP8), sk);
            bitArray p0 = split(p, 1), p1 = split(p, 2);
            return permute(join(get_sbox_bits(0, p0), get_sbox_bits(1, p1)), P4);
        }

        /** sk is the subkey **/
        bitArray Fk(bitArray b, bitArray sk){
            bitArray L = split(b, 1),
                     R = split(b ,2);
            return join(XOR(L, F(R, sk)), R);
        }

        bitArray sw(bitArray b){
            bitArray L = split(b, 1),
                     R = split(b ,2);
            return join(R, L);
        }

        bitArray byte2bitArray(byte b){
            bitArray res = dec2bin((int)b);
            stuff_bits(res, 8);
            return res;
        }

        byte bitArray2byte(bitArray b){
            byte res = 0;
            for(int i = 7, power2 = 1; i >= 0; --i, power2 <<= 1)
                res += power2 * (b[i] == 1? 1: 0);
            return res;
        }



        /***** Encrypt *****/
        byte encrypt(byte b){
            /*bitArray b1 = byte2bitArray(b);
            print(b1);
            bitArray b2 = permute(b1, IP);
            print(b2);
            bitArray b3 = Fk(b2, k1);
            print(b3);
            bitArray b4 = sw(b3);
            print(b4);
            bitArray b5 = Fk(b4, k2);
            print(b5);
            bitArray b6 = permute(b5, IP_inverse);
            print(b6);
            byte ret = bitArray2byte(b6);
            print(ret);
            return ret;*/
            return  bitArray2byte(permute( Fk( sw( Fk( permute(byte2bitArray(b), IP), k1)), k2), IP_inverse));
        }

        /***** Decrypt *****/
        byte decrypt(byte b){
            /*bitArray b1 = byte2bitArray(b);
            print(b1);
            bitArray b2 = permute(b1, IP);
            print(b2);
            bitArray b3 = Fk(b2, k2);
            print(b3);
            bitArray b4 = sw(b3);
            print(b4);
            bitArray b5 = Fk(b4, k1);
            print(b5);
            bitArray b6 = permute(b5, IP_inverse);
            print(b6);
            byte ret = bitArray2byte(b6);
            print(ret);
            return ret;*/
            return  bitArray2byte(permute( Fk( sw( Fk( permute(byte2bitArray(b), IP), k2)), k1), IP_inverse));
        }

    public:

        /** Constructor **/
        SDES(int master_key){
            set_key(master_key);
        }

	SDES(){}

	void set_key(int master_key){
	    if(!(master_key >= 0 && master_key < 1024))
                exit(1);
            bitArray m = dec2bin(master_key);
            stuff_bits(m, 10);
            generate_keys(m);
	}

        /** Encrypt a message **/
        string Encrypt(string msg){
            string res = "";
            for(char c: msg)
                res += encrypt((byte)c);
            return res;
        }

	void Encrypt(char* msg){
	    to_charArray(msg, Encrypt(to_string(msg)));
	}

        /** Decrypt a message **/
        string Decrypt(string msg){
            string res = "";
            for(char c: msg){
                res += decrypt((byte)c);
            }
            return res;
        }

	void Decrypt(char* msg){
	    to_charArray(msg, Decrypt(to_string(msg)));
	}

        /** Some utility functions for testing framework **/
        void print(byte b){
            print(byte2bitArray(b));
        }
        void print(bitArray b){
            for(int i = 0; i < length(b); ++i)
                printf((b[i] == 1? "1":"0"));
            printf("\n");
        }

    };

}

/***** Testing Framework *****/
namespace SDES_Test{
    string to_string(char *m){
        string res = "";
        for(int i = 0; i < strlen(m); ++i)
            res += m[i];
        return res;
    }
    string get_random_string(int len){
        string res = "";
        srand(time(NULL));
        for(int i = 0; i < len; ++i)
            res += (SDES_Project::byte)(rand() % 256);
        return res;
    }

    /** this function continually tests encryption and decryption and will report if any incorrect result is found **/
    void testSDES(){
        printf("Enter a key in range [0, 1023]:\t");
        int x;  string msg, cipher;
        scanf("%d", &x);    getchar();
        SDES_Project::SDES o(x);
        unsigned long long tested_cnt = 0;
        while(1){
            msg = get_random_string(100);
            o.Decrypt(cipher);
            if(msg != o.Decrypt(o.Encrypt(msg))){
                db("Incorrect", msg);
            }
            tested_cnt++;
            if(tested_cnt % 100 == 0)
                printf("%llu cases tested\n", tested_cnt);
        }
    }

    /** take input from user and encrypt / decrypt it **/
    void UI(){
        printf("Enter a key in range [0, 1023]:\t");
        int x;  string msg, cipher;
        char line[1024];
        scanf("%d", &x);    getchar();
        SDES_Project::SDES o(x);

        while(1){
            printf("\nMsg:\t");
            cin.getline(line, 1023, '\n');
            printf("You Entered:\t%s\n", line);
            printf("1. Encrypt \n2. Decrypt \nEnter choice:\t");
            int cmd;    scanf("%d", &cmd);  getchar();
            switch(cmd){
            case 1:
                o.Encrypt(line);
                printf("Encrypted Message:\t%s\n", line);
                break;
            case 2:
                o.Decrypt(line);
                printf("Decrypted Message:\t%s\n", line);
                break;
            }
        }
    }
}


