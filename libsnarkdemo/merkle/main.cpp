#define CURVE_ALT_BN128

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <boost/optional.hpp>
#include "../circuit/merklecircuit.h"
#include "../circuit/SampleEquationCircuit.h"
#include "../circuit/FishingBoatCircuit.h"

using namespace libsnark;


template<typename ppzksnark_ppT, typename FieldT, typename HashT>
r1cs_gg_ppzksnark_keypair<ppzksnark_ppT> generate_read_keypair(const size_t tree_depth)
{
    protoboard<FieldT> pb;

    sample::MerkleCircuit<FieldT, HashT> mc(pb, tree_depth);
    mc.generate_r1cs_constraints();
    r1cs_constraint_system<FieldT> cs = pb.get_constraint_system();

    std::cout << "Number of R1CS constraints: " << cs.num_constraints() << std::endl;

    return r1cs_gg_ppzksnark_generator<ppzksnark_ppT>(cs);
}

template<typename ppzksnark_ppT, typename FieldT, typename HashT>
boost::optional<r1cs_gg_ppzksnark_proof<ppzksnark_ppT>> generate_read_proof(r1cs_gg_ppzksnark_proving_key<ppzksnark_ppT> proving_key,
                                                                    const size_t tree_depth, libff::bit_vector& leaf,
                                                                    libff::bit_vector& root, merkle_authentication_path& path,
                                                                    const size_t address, libff::bit_vector& address_bits)
{
    protoboard<FieldT> pb;
    // 创建电路
    sample::MerkleCircuit<FieldT, HashT> mc(pb, tree_depth);
    // 生成R1CS
    mc.generate_r1cs_constraints();
    // 给所有变量进行赋值
    mc.generate_r1cs_witness(pb, leaf, root, path, address, address_bits);
    if (!pb.is_satisfied()) {
        std::cout << "pb is not satisfied" << std::endl;
        return boost::none;
    }

    return r1cs_gg_ppzksnark_prover<ppzksnark_ppT>(proving_key, pb.primary_input(), pb.auxiliary_input());
}

template<typename ppzksnark_ppT, typename FieldT>
bool verify_read_proof(r1cs_gg_ppzksnark_verification_key<ppzksnark_ppT> verification_key,
                  r1cs_gg_ppzksnark_proof<ppzksnark_ppT> proof, libff::bit_vector& root)
{
    r1cs_primary_input<FieldT> input;
    for (auto item : root) {
        input.push_back(FieldT(item));
    }
    return r1cs_gg_ppzksnark_verifier_strong_IC<ppzksnark_ppT>(verification_key, input, proof);
}

template<typename HashT>
boost::optional<std::string> binToHex(libff::bit_vector& bin) {
    if (bin.size() != HashT::get_digest_len() && bin.size() != HashT::get_digest_len() * 2) {
        std::cout << "The input binary input is not " << HashT::get_digest_len();
        return boost::none;
    }
    std::string res;
    for (int i = 0; i < bin.size(); i += 4) {
        std::string tmp;
        for (int j = i; j < i + 4; j++) {
            tmp.push_back(bin[j] == true ? '1' : '0');
        }
        if (tmp == "0000")
            res.push_back('0');
        else if (tmp == "0001")
            res.push_back('1');
        else if (tmp == "0010")
            res.push_back('2');
        else if (tmp == "0011")
            res.push_back('3');
        else if (tmp == "0100")
            res.push_back('4');
        else if (tmp == "0101")
            res.push_back('5');
        else if (tmp == "0110")
            res.push_back('6');
        else if (tmp == "0111")
            res.push_back('7');
        else if (tmp == "1000")
            res.push_back('8');
        else if (tmp == "1001")
            res.push_back('9');
        else if (tmp == "1010")
            res.push_back('a');
        else if (tmp == "1011")
            res.push_back('b');
        else if (tmp == "1100")
            res.push_back('c');
        else if (tmp == "1101")
            res.push_back('d');
        else if (tmp == "1110")
            res.push_back('e');
        else if (tmp == "1111")
            res.push_back('f');
    }
    return res;
}

std::string hexToChar(const char c) {
    switch(tolower(c))
    {
        case '0': return "0000";
        case '1': return "0001";
        case '2': return "0010";
        case '3': return "0011";
        case '4': return "0100";
        case '5': return "0101";
        case '6': return "0110";
        case '7': return "0111";
        case '8': return "1000";
        case '9': return "1001";
        case 'a': return "1010";
        case 'b': return "1011";
        case 'c': return "1100";
        case 'd': return "1101";
        case 'e': return "1110";
        case 'f': return "1111";
    }
}

libff::bit_vector hexToBin(std::string& str) {
    libff::bit_vector res;
    for (auto item : str) {
        std::string hexItem = hexToChar(item);
        res.push_back(hexItem[0] == '1' ? true : false);
        res.push_back(hexItem[1] == '1' ? true : false);
        res.push_back(hexItem[2] == '1' ? true : false);
        res.push_back(hexItem[3] == '1' ? true : false);
    }
    return res;
}

std::vector<std::string> split(std::string& str, std::string delim) {
    std::vector<std::string> res;
    auto start = 0U;
    auto end = str.find(delim);
    while (end != std::string::npos)
    {
        std::cout << str.substr(start, end - start) << std::endl;
        res.push_back(str.substr(start, end - start));
        start = end + delim.length();
        end = str.find(delim, start);
    }
    return res;
}

template<typename HashT>
libff::bit_vector hash256(std::string str) {
    libff::bit_vector operand;
    for (int i = 0; i < str.size(); i++) {
        char tmpc[5];
        sprintf(tmpc, "%x", str[i]);
        std::string tmps(tmpc);
        libff::bit_vector s = hexToBin(tmps);
        operand.insert(operand.end(), s.begin(), s.end());
    }
    //padding input
    size_t size = operand.size();
    char tmpc[20];
    sprintf(tmpc, "%x", size);
    std::string tmps(tmpc);
    libff::bit_vector s = hexToBin(tmps);
    operand.push_back(1);
    for (int i = size + 1; i < HashT::get_block_len() - s.size(); i++) {
        operand.push_back(0);
    }
    operand.insert(operand.end(), s.begin(), s.end());
    libff::bit_vector res = HashT::get_hash(operand);
    return res;
}

template<typename HashT>
void calcAllLevels(std::vector<std::vector<libff::bit_vector>>& levels, size_t level) {
    //level 1 upper layer
    for (int i = level; i > 0; i--) {
        for (int j = 0; j < levels[i].size(); j += 2) {
            libff::bit_vector input = levels[i][j];
            // 存储剩余三个标记为，自身已有input begin，在加上input的end，下一个节点的开始与结束
            input.insert(input.end(), levels[i][j+1].begin(), levels[i][j+1].end());
            levels[i-1].push_back(HashT::get_hash(input));
        }
    }
}

void merkleGenerate(char* argv[], size_t tree_depth) {
    typedef libff::default_ec_pp ppzksnark_ppT;
    typedef libff::Fr<ppzksnark_ppT> FieldT;
    typedef sha256_two_to_one_hash_gadget<FieldT> HashT;

    const size_t digest_len = HashT::get_digest_len();
    std::vector<merkle_authentication_node> path(tree_depth);

    libff::bit_vector prev_hash(digest_len);
    std::generate(prev_hash.begin(), prev_hash.end(), [&]() { return std::rand() % 2; });
    libff::bit_vector leaf = prev_hash;

    libff::bit_vector address_bits;

    size_t address = 0;
    for (long level = tree_depth-1; level >= 0; --level)
    {
        const bool computed_is_right = (std::rand() % 2);
        address |= (computed_is_right ? 1ul << (tree_depth-1-level) : 0);
        address_bits.push_back(computed_is_right);
        libff::bit_vector other(digest_len);
        std::generate(other.begin(), other.end(), [&]() { return std::rand() % 2; });

        libff::bit_vector block = prev_hash;
        block.insert(computed_is_right ? block.begin() : block.end(), other.begin(), other.end());
        libff::bit_vector h = HashT::get_hash(block);

        std::cout << *(binToHex<HashT>(block)) << std::endl;
        std::cout  << "h: " << *(binToHex<HashT>(h)) << std::endl;
        path[level] = other;

        prev_hash = h;
    }
    libff::bit_vector root = prev_hash;

    auto hexLeaf = binToHex<HashT>(leaf);
    std::fstream mk("merkle.txt", std::ios_base::out);
    mk << "leaf: " << *hexLeaf << std::endl; //Write out leaf
    mk << "index: " << address << std::endl; //Write out index
    mk << "path: ";                          //Write out path
    for (int i = 0; i < path.size(); i++)
        mk << *(binToHex<HashT>(path[i])) << " ";
    mk << std::endl;
    mk << "root: " << *(binToHex<HashT>(root));      //Write out root
}

void merkleSetup(char* argv[], size_t tree_depth) {
    typedef libff::default_ec_pp ppzksnark_ppT;
    typedef libff::Fr<ppzksnark_ppT> FieldT;
    typedef sha256_two_to_one_hash_gadget<FieldT> HashT;

    std::cout << "in setup" << std::endl;
    // 准备CRS
    auto keypair = generate_read_keypair<ppzksnark_ppT, FieldT, HashT>(tree_depth);
    std::fstream pk("merkle_pk.raw", std::ios_base::out);
    pk << keypair.pk;
    pk.close();
    std::fstream vk("merkle_vk.raw", std::ios_base::out);
    vk << keypair.vk;
    vk.close();
}

void merkleProve(char* argv[], size_t tree_depth) {
    typedef libff::default_ec_pp ppzksnark_ppT;
    typedef libff::Fr<ppzksnark_ppT> FieldT;
    typedef sha256_two_to_one_hash_gadget<FieldT> HashT;

    // 生成证明
    //load pk，读取公共字符串
    std::fstream f_pk("merkle_pk.raw", std::ios_base::in);
    r1cs_gg_ppzksnark_proving_key<ppzksnark_ppT> pk;
    f_pk >> pk;
    f_pk.close();

    libff::bit_vector leaf, root, address_bits(tree_depth);
    size_t address;
    std::vector<merkle_authentication_node> path(tree_depth);

    //generate witness depth几层就有几个level
    std::vector<std::vector<libff::bit_vector>> levels(tree_depth);
    //level 2 leaves left most --> right most
    // 计算最后一层节点数量
    int leaf_count = std::pow(2, tree_depth);
    // 先最后一层的节点的hash，为什么只计算最后一层，下面会从最后一层开始网上推导上一层的hash
    for (int i = 0; i < leaf_count; i++) {
        libff::bit_vector tmp = hash256<HashT>(argv[i+2]);
        //std::cout << *binToHex<HashT>(tmp) << std::endl;
        levels[tree_depth - 1].push_back(tmp);
    }
    // 从最后一层开始向上推导节点的hash
    calcAllLevels<HashT>(levels, tree_depth-1);
    // 计算root节点的hash
    libff::bit_vector input = levels[0][0];
    input.insert(input.end(), levels[0][1].begin(), levels[0][1].end());
    root = HashT::get_hash(input);

    // 获取某一个叶子节点，例如当前tree depth是3，那么address可以取值0-7
    address = std::stoi(argv[10]);
    leaf = levels[tree_depth-1][address];
    std::cout << address << std::endl;
    int addr = address;
    // TODO 此步没有看太懂，看起来是一层层计算节点处于左边还是右边
    for (int i = 0; i < tree_depth; i++) {
        // 取右边第一位数字
        int tmp = (addr & 0x01);
        address_bits[i] = tmp;
        // 类似左移一位
        addr = addr / 2;
        std::cout << address_bits[tree_depth-1-i] << std::endl;
    }

    //Fill in the path
    size_t index = address;
    for (int i = tree_depth - 1; i >= 0; i--) {
        path[i] = address_bits[tree_depth-1-i] == 0 ? levels[i][index+1] : levels[i][index-1];
        index = index / 2;
    }
    std::cout << "root is " << *binToHex<HashT>(root) << std::endl;

//generate proof
    auto proof = generate_read_proof<ppzksnark_ppT, FieldT, HashT>(
            pk, tree_depth, leaf, root, path, address, address_bits);
    if (proof != boost::none) {
        std::cout << "Proof generated!" << std::endl;
    }

//save the proof
    std::fstream pr("proof.raw", std::ios_base::out);
    pr << (*proof);
    pr.close();
}

void merkleVerify(char* argv[]) {
    typedef libff::default_ec_pp ppzksnark_ppT;
    typedef libff::Fr<ppzksnark_ppT> FieldT;
    typedef sha256_two_to_one_hash_gadget<FieldT> HashT;

    //load proof
    std::fstream pr("proof.raw", std::ios_base::in);
    r1cs_gg_ppzksnark_proof<ppzksnark_ppT> proof;
    pr >> proof;
    pr.close();
//load vk
    std::fstream vkf("merkle_vk.raw", std::ios_base::in);
    r1cs_gg_ppzksnark_verification_key<ppzksnark_ppT> vk;
    vkf >> vk;
    vkf.close();

//load root
    std::string r(argv[2]);
    libff::bit_vector root = hexToBin(r);
//verify the proof
    bool ret = verify_read_proof<ppzksnark_ppT, FieldT>(vk, proof, root);
    if (ret) {
        std::cout << "Verification pass!" << std::endl;
    } else {
        std::cout << "Verification failed!" << std::endl;
    }
}

void equationSetup() {
    typedef libff::default_ec_pp ppzksnark_ppT;
    typedef libff::Fr<ppzksnark_ppT> FieldT;

    std::cout << "in equation setup" << std::endl;
    protoboard<FieldT> pb;

    sample::EquationCircuit ec(pb);
    auto keypair = ec.setup(pb);

    std::fstream pk("equation_pk.raw", std::ios_base::out);
    pk << keypair.pk;
    pk.close();
    std::fstream vk("equation_vk.raw", std::ios_base::out);
    vk << keypair.vk;
    vk.close();
}

void equationProve() {
    typedef libff::default_ec_pp ppzksnark_ppT;
    typedef libff::Fr<ppzksnark_ppT> FieldT;

    std::cout << "in equation prove" << std::endl;

    //load pk，读取公共字符串
    std::fstream f_pk("equation_pk.raw", std::ios_base::in);
    r1cs_gg_ppzksnark_proving_key<ppzksnark_ppT> pk;
    f_pk >> pk;
    f_pk.close();

    protoboard<FieldT> pb;
    sample::EquationCircuit ec(pb);

    ec.generate_r1cs_constraints(pb);
    ec.generate_r1cs_witness(pb);
    if (!pb.is_satisfied()) {
        std::cout << "pb is not satisfied" << std::endl;
        return;
    }

    // 调用r1cs_gg_ppzksnark_prover方法进行proof的生成，输入参数为 CRS中的PK、public input、private witness。
    // 此处可以明显看到，并没有明确的输入电路板信息，唯一可能与电路相关联的就是keypair.pk
    const r1cs_gg_ppzksnark_proof<default_r1cs_gg_ppzksnark_pp> proof = r1cs_gg_ppzksnark_prover<default_r1cs_gg_ppzksnark_pp>(pk, pb.primary_input(), pb.auxiliary_input());

    //save the proof
    std::fstream pr("equation_proof.raw", std::ios_base::out);
    pr << (proof);
    pr.close();
}

void equationVerify() {
    typedef libff::default_ec_pp ppzksnark_ppT;
    typedef libff::Fr<ppzksnark_ppT> FieldT;

    std::cout << "in equation verify" << std::endl;
    //load proof
    std::fstream pr("equation_proof.raw", std::ios_base::in);
    r1cs_gg_ppzksnark_proof<ppzksnark_ppT> proof;
    pr >> proof;
    pr.close();
    //load vk
    std::fstream vkf("equation_vk.raw", std::ios_base::in);
    r1cs_gg_ppzksnark_verification_key<ppzksnark_ppT> vk;
    vkf >> vk;
    vkf.close();

    // 准备public input
    r1cs_primary_input<FieldT> input;
    input.push_back(FieldT(35));

    // 调用r1cs_gg_ppzksnark_verifier_strong_IC方法进行proof校验，输入参数为 CRS中的VK、public input、proof
    bool verified = r1cs_gg_ppzksnark_verifier_strong_IC<default_r1cs_gg_ppzksnark_pp>(vk, input, proof);
    if (verified) {
        std::cout << "Verification pass!" << std::endl;
    } else {
        std::cout << "Verification failed!" << std::endl;
    }
}

void fishingBoatSetup() {
    typedef libff::default_ec_pp ppzksnark_ppT;
    typedef libff::Fr<ppzksnark_ppT> FieldT;

    std::cout << "in fishingBoat setup" << std::endl;
    protoboard<FieldT> pb;

    sample::FishingBoatCircuit fbc(pb);
    auto keypair = fbc.setup(pb);

    std::fstream pk("fishingBoat_pk.raw", std::ios_base::out);
    pk << keypair.pk;
    pk.close();
    std::fstream vk("fishingBoat_vk.raw", std::ios_base::out);
    vk << keypair.vk;
    vk.close();
}

void fishingBoatProve() {
    typedef libff::default_ec_pp ppzksnark_ppT;
    typedef libff::Fr<ppzksnark_ppT> FieldT;

    std::cout << "in fishingBoat prove" << std::endl;

    //load pk，读取公共字符串
    std::fstream f_pk("fishingBoat_pk.raw", std::ios_base::in);
    r1cs_gg_ppzksnark_proving_key<ppzksnark_ppT> pk;
    f_pk >> pk;
    f_pk.close();

    std::cout << "in fishingBoat prove start new fbc" << std::endl;
    protoboard<FieldT> pb;
    sample::FishingBoatCircuit fbc(pb);

    std::cout << "in fishingBoat prove before generate_r1cs_constraints" << std::endl;
    fbc.generate_r1cs_constraints(pb);
    std::cout << "in fishingBoat prove before generate_r1cs_witness" << std::endl;
    fbc.generate_r1cs_witness(pb);
    if (!pb.is_satisfied()) {
        std::cout << "pb is not satisfied" << std::endl;
        return;
    }

    std::cout << "in fishingBoat prove before r1cs_gg_ppzksnark_prover" << std::endl;
    // 调用r1cs_gg_ppzksnark_prover方法进行proof的生成，输入参数为 CRS中的PK、public input、private witness。
    // 此处可以明显看到，并没有明确的输入电路板信息，唯一可能与电路相关联的就是keypair.pk
    const r1cs_gg_ppzksnark_proof<default_r1cs_gg_ppzksnark_pp> proof = r1cs_gg_ppzksnark_prover<default_r1cs_gg_ppzksnark_pp>(pk, pb.primary_input(), pb.auxiliary_input());

    std::cout << "in fishingBoat prove before write fishingBoat_proof" << std::endl;
    //save the proof
    std::fstream pr("fishingBoat_proof.raw", std::ios_base::out);
    pr << (proof);
    pr.close();
}

void fishingBoatVerify() {
    typedef libff::default_ec_pp ppzksnark_ppT;
    typedef libff::Fr<ppzksnark_ppT> FieldT;

    std::cout << "in fishingBoat verify" << std::endl;
    //load proof
    std::fstream pr("fishingBoat_proof.raw", std::ios_base::in);
    r1cs_gg_ppzksnark_proof<ppzksnark_ppT> proof;
    pr >> proof;
    pr.close();
    //load vk
    std::fstream vkf("fishingBoat_vk.raw", std::ios_base::in);
    r1cs_gg_ppzksnark_verification_key<ppzksnark_ppT> vk;
    vkf >> vk;
    vkf.close();

    // 准备public input
    r1cs_primary_input<FieldT> input;
    input.push_back(FieldT(30));
    input.push_back(FieldT(60));
    input.push_back(FieldT(40));
    input.push_back(FieldT(60));
    input.push_back(FieldT(4));

    // 调用r1cs_gg_ppzksnark_verifier_strong_IC方法进行proof校验，输入参数为 CRS中的VK、public input、proof
    bool verified = r1cs_gg_ppzksnark_verifier_strong_IC<default_r1cs_gg_ppzksnark_pp>(vk, input, proof);
    if (verified) {
        std::cout << "Verification pass!" << std::endl;
    } else {
        std::cout << "Verification failed!" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    libff::default_ec_pp::init_public_params();

    const size_t tree_depth = 3;
    if (std::string(argv[1]) == "equationSetup") {
        equationSetup();
    } else if (std::string(argv[1]) == "equationProve") {
        equationProve();
    } else if (std::string(argv[1]) == "equationVerify") {
        equationVerify();
    } else if (std::string(argv[1]) == "fishingBoatSetup") {
        fishingBoatSetup();
    } else if (std::string(argv[1]) == "fishingBoatProve") {
        fishingBoatProve();
    } else if (std::string(argv[1]) == "fishingBoatVerify") {
        fishingBoatVerify();
    } else if (std::string(argv[1]) == "generate") {
        merkleGenerate(argv, tree_depth);
    } else if (std::string(argv[1]) == std::string("setup")) {
        merkleSetup(argv, tree_depth);
    } else if (std::string(argv[1]) == std::string("prove")) {
        merkleProve(argv, tree_depth);
    } else if (std::string(argv[1]) == std::string("verify")) {
	    merkleVerify(argv);
    }

    return 0;
}
