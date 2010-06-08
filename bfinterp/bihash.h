/*
Copyright 2010 Tom Eklof. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY TOM EKLOF ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL TOM EKLOF OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef BIHASH_H
#define BIHASH_H
#include <QHash>
#include <QList>

namespace QtBrain {

    /**
      This is a bidirectional hash which makes lookup by key and by value much easier.
      Instead of doing O(n) key lookups using QHash::keys(), two QHashes are used to
      store the values. This way the speed of looking up by either "side" of the hash is
      the same as QHash's amortized O(1).

      This particular implementation requires that all keys and values be unique, so storing
      something like (5,7) and (7,2) won't work.
      */

    template <typename K, typename V>
            class BiHash
    {
    public:
        BiHash()  {}

        ~BiHash() {
        }

        /**
          "standard" QHash value lookup, ie. looks up a value associated with a key
          */
        const V value(const K &k) const{
            return m_lhash.value(k);
        }
        /**
          looks up the key associated with the value.
          */
        const K key(const V &k) const {
            return m_rhash.value(k);
        }

        /**
          returns true if the BiHash contains the key k
          */
        bool containsKey(const K &k) const {
            return m_lhash.contains(k);
        }


        /**
          returns the "left" QHash, ie. the one with "<Key,Value>" mapping
          */
        const QHash<K,V> lhash() const  {
            return m_lhash;
        }

        /**
          returns the "right" QHash, ie. the one with "<Value,Key>" mapping
          */
        const QHash<V,K> rhash() const {
            return m_rhash;
        }

        /**
          inserts a key and an associated value. All keys and values must be unique,
          and this is enforced by insert(). No exception is thrown if they are not unique,
          the old keys and values are simply replaced.
          */
        void insert(const K &k, const V &v) {

            /* if the key is in the "left" hash, remove it from the both the "left"
               AND the "right" hashes.
               For example if (15,7) had been inserted, the hashes would contain:
               lhash: (15,7)
               rhash: (7,15)
               trying to insert (15,9) would cause a collision since 15 is already "mapped
               to" 7. All keys and values containing 15 must be removed.
               */
            if(m_lhash.contains(k)) {
                m_rhash.remove(m_lhash.value(k));
                m_lhash.remove(k);
            }

            /* the same applies to the "right" hash.
               Example:
               (15,7) inserted
               lhash: (15,7)
               rhash: (7,15)
               trying to insert (1,7) causes a collision and all keys and values that contain
               7 have to be removed.
               */

            if(m_rhash.contains(v)) {
                m_lhash.remove(m_rhash.value(v));
                m_rhash.remove(v);
            }

            m_lhash.insert(k,v);
            m_rhash.insert(v,k);
            Q_ASSERT_X(m_lhash.size() == m_rhash.size(), "BiHash::insert()",
                       "hash sizes don't match");
        }

        /**
          Returns true if the BiHash is empty, false otherwise.
          */
        bool isEmpty() const {
            // No need to check both hashes since they're symmetrical anyhow.
            return m_lhash.isEmpty();
        }



    protected:
        QHash<K,V> m_lhash;
        QHash<V,K> m_rhash;
    };
}


#endif // BIHASH_H
