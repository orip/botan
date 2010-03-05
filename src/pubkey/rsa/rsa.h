/*
* RSA
* (C) 1999-2008 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_RSA_H__
#define BOTAN_RSA_H__

#include <botan/if_algo.h>

namespace Botan {

/**
* RSA Public Key
*/
class BOTAN_DLL RSA_PublicKey : public PK_Encrypting_Key,
                                public PK_Verifying_with_MR_Key,
                                public virtual IF_Scheme_PublicKey
   {
   public:
      std::string algo_name() const { return "RSA"; }

      SecureVector<byte> encrypt(const byte[], u32bit,
                                 RandomNumberGenerator& rng) const;

      RSA_PublicKey(const AlgorithmIdentifier& alg_id,
                    const MemoryRegion<byte>& key_bits) :
         IF_Scheme_PublicKey(alg_id, key_bits)
         {
         core = IF_Core(e, n);
         }

      /**
      * Create a RSA_PublicKey
      * @arg n the modulus
      * @arg e the exponent
      */
      RSA_PublicKey(const BigInt& n, const BigInt& e) :
         IF_Scheme_PublicKey(n, e)
         {
         core = IF_Core(e, n);
         }

   protected:
      RSA_PublicKey() {}
      BigInt public_op(const BigInt&) const;
   };

/**
* RSA Private Key class.
*/
class BOTAN_DLL RSA_PrivateKey : public RSA_PublicKey,
                                 public PK_Decrypting_Key,
                                 public PK_Signing_Key,
                                 public IF_Scheme_PrivateKey
   {
   public:
      SecureVector<byte> decrypt(const byte[], u32bit) const;

      bool check_key(RandomNumberGenerator& rng, bool) const;

      RSA_PrivateKey(const AlgorithmIdentifier& alg_id,
                     const MemoryRegion<byte>& key_bits,
                     RandomNumberGenerator& rng) :
         IF_Scheme_PrivateKey(rng, alg_id, key_bits) {}

      /**
      * Construct a private key from the specified parameters.
      * @param rng a random number generator
      * @param p the first prime
      * @param q the second prime
      * @param e the exponent
      * @param d if specified, this has to be d with
      * exp * d = 1 mod (p - 1, q - 1). Leave it as 0 if you wish to
      * the constructor to calculate it.
      * @param n if specified, this must be n = p * q. Leave it as 0
      * if you wish to the constructor to calculate it.
      */
      RSA_PrivateKey(RandomNumberGenerator& rng,
                     const BigInt& p, const BigInt& q,
                     const BigInt& e, const BigInt& d = 0,
                     const BigInt& n = 0) :
         IF_Scheme_PrivateKey(rng, p, q, e, d, n) {}

      /**
      * Create a new private key with the specified bit length
      * @param rng the random number generator to use
      * @param bits the desired bit length of the private key
      * @param exp the public exponent to be used
      */
      RSA_PrivateKey(RandomNumberGenerator& rng,
                     u32bit bits, u32bit exp = 65537);
   private:
      BigInt private_op(const byte[], u32bit) const;
   };

class BOTAN_DLL RSA_Private_Operation : public PK_Ops::Signature,
                                        public PK_Ops::Decryption
   {
   public:
      RSA_Private_Operation(const RSA_PrivateKey& rsa);

      u32bit max_input_bits() const { return (n.bits() - 1); }

      SecureVector<byte> sign(const byte msg[], u32bit msg_len,
                              RandomNumberGenerator& rng) const;

      SecureVector<byte> decrypt(const byte msg[], u32bit msg_len) const;

   private:
      BigInt private_op(const BigInt& m) const;

      const BigInt& n;
      const BigInt& q;
      const BigInt& c;
      Fixed_Exponent_Power_Mod powermod_d1_p, powermod_d2_q;
      Modular_Reducer mod_p;
      u32bit n_bits;
   };

class BOTAN_DLL RSA_Public_Operation : public PK_Ops::Verification,
                                       public PK_Ops::Encryption
   {
   public:
      RSA_Public_Operation(const RSA_PublicKey& rsa) :
         n(rsa.get_n()), powermod_e_n(rsa.get_e(), rsa.get_n())
         {}

      u32bit max_input_bits() const { return (n.bits() - 1); }
      bool with_recovery() const { return true; }

      SecureVector<byte> encrypt(const byte msg[], u32bit msg_len,
                                 RandomNumberGenerator&) const
         {
         BigInt m(msg, msg_len);
         return BigInt::encode_1363(public_op(m), n.bytes());
         }

      SecureVector<byte> verify_mr(const byte msg[], u32bit msg_len) const
         {
         BigInt m(msg, msg_len);
         return BigInt::encode(public_op(m));
         }

   private:
      BigInt public_op(const BigInt& m) const
         {
         if(m >= n)
            throw Invalid_Argument("RSA public op - input is too large");
         return powermod_e_n(m);
         }

      const BigInt& n;
      Fixed_Exponent_Power_Mod powermod_e_n;
   };

}

#endif
