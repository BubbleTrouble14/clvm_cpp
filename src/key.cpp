#include "key.h"

#include <schemes.hpp>

#include "mnemonic.h"
#include "utils.h"

namespace chia {
namespace wallet {

bool Key::VerifySig(PublicKey const& pub_key, Bytes const& msg,
                    Signature const& sig) {
  return bls::AugSchemeMPL().Verify(utils::bytes_cast<PUB_KEY_LEN>(pub_key),
                                    msg, utils::bytes_cast<SIG_LEN>(sig));
}

Key::Key() {}

Key::Key(PrivateKey priv_key) : priv_key_(std::move(priv_key)) {}

Key::Key(Mnemonic const& mnemonic, std::string_view passphrase) {
  Bytes64 seed = mnemonic.GetSeed(passphrase);
  priv_key_ = utils::bytes_cast<PRIV_KEY_LEN>(
      bls::AugSchemeMPL().KeyGen(utils::bytes_cast<64>(seed)).Serialize());
}

bool Key::IsEmpty() const { return priv_key_.empty(); }

void Key::GenerateNew(Bytes const& seed) {
  bls::PrivateKey bls_priv_key = bls::AugSchemeMPL().KeyGen(seed);
  Bytes priv_key_bytes = bls_priv_key.Serialize();
  priv_key_ = utils::bytes_cast<PRIV_KEY_LEN>(priv_key_bytes);
}

PrivateKey Key::GetPrivateKey() const { return priv_key_; }

PublicKey Key::GetPublicKey() const {
  bls::PrivateKey bls_priv_key = bls::PrivateKey::FromBytes(
      bls::Bytes(utils::bytes_cast<PRIV_KEY_LEN>(priv_key_)));
  return utils::bytes_cast<PUB_KEY_LEN>(
      bls_priv_key.GetG1Element().Serialize());
}

Signature Key::Sign(Bytes const& msg) {
  bls::PrivateKey bls_priv_key = bls::PrivateKey::FromBytes(
      bls::Bytes(utils::bytes_cast<PRIV_KEY_LEN>(priv_key_)));
  Bytes sig_bytes = bls::AugSchemeMPL().Sign(bls_priv_key, msg).Serialize();
  return utils::bytes_cast<SIG_LEN>(sig_bytes);
}

Key Key::DerivePath(std::vector<uint32_t> const& paths) const {
  bls::PrivateKey bls_priv_key = bls::PrivateKey::FromBytes(
      bls::Bytes(utils::bytes_cast<PRIV_KEY_LEN>(priv_key_)));
  auto sk{bls_priv_key};
  for (uint32_t path : paths) {
    sk = bls::AugSchemeMPL().DeriveChildSk(sk, path);
  }
  return Key(utils::bytes_cast<PRIV_KEY_LEN>(sk.Serialize()));
}

}  // namespace wallet
}  // namespace chia