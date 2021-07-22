use std::io::{BufReader, Read};

/// Workload header of the ycsb benchmark implementation for M3.
#[allow(dead_code)]
pub struct WorkloadHeader {
    /// A workload is split in two phases.
    /// 1. Inserts, that build the database
    /// 2. Actual benchmarking operations
    ///
    /// This is the number of preparing inserts. After this the actual benchmarking operations will
    /// start. You can use this information to either benchmark the whole workload, or just the
    /// operation part of the benchmark.
    pub number_of_preinserts: u32,
    /// Number of operations including inserts and benchmarking operations.
    pub number_of_operations: u32,
}

impl WorkloadHeader {
    #[allow(dead_code)]
    pub fn load_from_file<R: Read>(reader: &mut BufReader<R>) -> Self {
        let mut number_of_preinserts = [0 as u8; 4];
        reader.read(&mut number_of_preinserts).unwrap();
        let number_of_preinserts = u32::from_be_bytes(number_of_preinserts);

        let mut number_of_operations = [0 as u8; 4];
        reader.read(&mut number_of_operations).unwrap();
        let number_of_operations = u32::from_be_bytes(number_of_operations);
        WorkloadHeader {
            number_of_preinserts,
            number_of_operations,
        }
    }
}

/// Database operations. They usually use the `key` field to determine on which record is being worked on.
/// The only expection is `scan` which uses the `key` field as "start of scan" and the "scan_length" field
/// to determine how many records are scanned.
#[allow(dead_code)]
pub enum DbOp {
    Insert = 1,
    Delete = 2,
    Read   = 3,
    Scan   = 4,
    Update = 5,
}

#[allow(dead_code)]
pub enum Table {
    Usertable,
}

/// A single database operation.
#[derive(Debug)]
#[repr(C)]
pub struct Package {
    /// Operation on that package
    pub op: u8,
    /// Table to work on. Should always be 0
    pub table: u8,
    /// Key of the record in `table`
    pub key: u64,
    /// If an scan op, number of keys starting at `key` to scan.
    pub scan_length: u64,
    /// If `len()` is 0, this means "everything". So a Delete with `kv_pairs.len() == 0` means
    /// "delete whole record". If it has a length, the kv_pairs need to be read and worked on. The
    /// Keys are `field0`..`field9` usually. However can be more depending on the YCSB
    /// configuration. The values are long garbage strings.
    pub kv_pairs: Vec<(String, String)>,
}

impl Package {
    /// assumes that the cursor is on the start of an header
    #[allow(dead_code)]
    fn load<R: Read>(reader: &mut BufReader<R>) -> Self {
        let mut header = [0 as u8; 19];
        reader.read_exact(&mut header).unwrap();

        let mut u64_buf = [0 as u8; 8];
        u64_buf.copy_from_slice(&header[3..11]);
        let key = u64::from_be_bytes(u64_buf);

        let mut u64_buf = [0 as u8; 8];
        u64_buf.copy_from_slice(&header[11..19]);
        let scan_length = u64::from_be_bytes(u64_buf);

        let num_kvs = header[2] as usize;

        let mut kv_pairs = Vec::with_capacity(num_kvs);
        // Now read all key_value_pairs
        for _ in 0..num_kvs {
            let mut length = [0 as u8; 2];
            reader.read_exact(&mut length).unwrap();

            let mut key = String::with_capacity(length[0] as usize);
            let mut value = String::with_capacity(length[1] as usize);

            reader
                .take(length[0] as u64)
                .read_to_string(&mut key)
                .unwrap();
            reader
                .take(length[1] as u64)
                .read_to_string(&mut value)
                .unwrap();
            kv_pairs.push((key, value))
        }

        Package {
            op: header[0],
            table: header[1],
            key,
            scan_length,
            kv_pairs,
        }
    }

    /// Only loads the header information of this package and returns the whole package as byte
    /// buffer. Used to read a package from a byte stream, without parsing it into the actual
    /// format.
    #[allow(dead_code)]
    pub fn load_as_bytes<R: Read>(reader: &mut BufReader<R>) -> Vec<u8> {
        // Read static sized data into bytes vec
        let mut bytes = Vec::with_capacity(19);
        // safety: we initialize all bytes below
        unsafe {
            bytes.set_len(19);
        }
        reader.read_exact(&mut bytes).unwrap();

        for _i in 0..(bytes[2] as usize) {
            let mut length = [0 as u8; 2];
            reader.read_exact(&mut length).unwrap();

            bytes.push(length[0]);
            bytes.push(length[1]);

            let off = bytes.len();
            let add = length[0] as usize + length[1] as usize;
            bytes.reserve_exact(add);
            // safety: we initialize all bytes below
            unsafe {
                bytes.set_len(off + add);
            }
            reader.read_exact(&mut bytes[off..]).unwrap();
        }

        bytes
    }

    /// Interpretes the `data` slice as a Package. Returns Err(false) if 'data' was too short.
    /// Returns Err(true) if some information does not add up, for instance if a string can't be
    /// parsed. Returns true with the (size, Package) where size is the number of bytes that have
    /// been read.
    #[allow(dead_code)]
    pub fn from_bytes(data: &[u8]) -> Result<(usize, Self), bool> {
        if data.len() < 19 {
            return Err(false);
        }
        let mut u64_buf = [0 as u8; 8];
        u64_buf.copy_from_slice(&data[3..11]);
        let key = u64::from_be_bytes(u64_buf);

        let mut u64_buf = [0 as u8; 8];
        u64_buf.copy_from_slice(&data[11..19]);
        let scan_length = u64::from_be_bytes(u64_buf);

        let num_kvs = data[2] as usize;

        let mut kv_pairs = Vec::with_capacity(num_kvs);
        // Now read all key_value_pairs
        let mut data_ptr = 19;
        for _i in 0..num_kvs {
            if (data_ptr + 2) > data.len() {
                return Err(false);
            }
            let length = [data[data_ptr], data[data_ptr + 1]];
            // check that the length is within the parameters
            if (data_ptr + length[0] as usize + length[1] as usize + 2) > data.len() {
                return Err(false);
            }

            data_ptr += 2;
            let key = match core::str::from_utf8(&data[data_ptr..data_ptr + length[0] as usize]) {
                Ok(st) => st,
                Err(e) => {
                    println!("1st. String error: {}", e);
                    return Err(true);
                },
            };

            data_ptr += length[0] as usize;
            let value = match core::str::from_utf8(&data[data_ptr..data_ptr + length[1] as usize]) {
                Ok(st) => st,
                Err(e) => {
                    println!("2nd. String error: {}", e);
                    return Err(true);
                },
            };
            data_ptr += length[1] as usize;

            kv_pairs.push((key.to_string(), value.to_string()));
        }

        Ok((data_ptr, Package {
            op: data[0],
            table: data[1],
            key,
            scan_length,
            kv_pairs,
        }))
    }
}

impl PartialEq for Package {
    fn eq(&self, other: &Self) -> bool {
        if self.op != other.op || self.table != other.table || self.key != other.key {
            return false;
        }

        for ((k, v), (ok, ov)) in self.kv_pairs.iter().zip(other.kv_pairs.iter()) {
            if k != ok || v != ov {
                return false;
            }
        }

        true
    }
}
